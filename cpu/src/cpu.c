#include <cpu.h>

t_log* logger;
t_config* config;
t_registros* registros;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

int main(int argc, char* argv[]) {

	logger = iniciar_logger("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);
	char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

	registros = inicializarRegistros();

	t_socket server_fd = iniciar_servidor(puerto, logger);
	free(puerto);

	while(1){
		t_socket socket_cliente = esperar_cliente(server_fd, logger);
		if(socket_cliente != -1){
			t_pcb* pcb = inicializar_pcb();
			int cod_op = recibir_operacion(socket_cliente);
			if(cod_op == PROCESO) {
				pcb = recibir_proceso(socket_cliente);
				realizar_ciclo_instruccion(pcb);
				log_info(logger, "Enviando pcb a kernel");
				actualizar_registros_pcb(pcb);
				enviar_pcb(pcb,socket_cliente);

			} else {
				send(socket_cliente, (void *)(intptr_t)RESULT_ERROR, sizeof(uint32_t), (intptr_t)NULL);
				log_error(logger,"No me llego un proceso");

			}
		}
	}
}


t_pcb* recibir_proceso(int socket_cliente) {
	    int size;
		int desplazamiento = 0;
		void * buffer;
		t_list* valores = list_create();
		int tamanio;
        t_pcb* pcb = inicializar_pcb();
		t_list* instrucciones = list_create();

		//printf("hola, recibiendo proceso\n");

		//printf("recibiendo buffer\n");

		buffer = recibir_buffer(&size, socket_cliente);

		//recibir_variable(&pid,buffer,&desplazamiento);      // recibo pid
		//pcb->pid = pid;                                     // actualizo pid en el pcb
		memcpy(&(pcb->pid), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		/*recibir_variable(&pc,buffer,&desplazamiento);       //recivo pc
		pcb->pc = pc;                                      // actualizo pc en el pcb
*/
		memcpy(&(pcb->pc), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		/* alternativa con vectores por tamaño
		recibir_registros(buffer,&desplazamiento, 4,registros_>tamanio_4);
		pcb->registros->tamanio_4 = registros->tamanio_4;
		recibir_registros(buffer,&desplazamiento, 8,registros_>tamanio_8);
		pcb->registros->tamanio_8 = registros->tamanio_8;
		recibir_registros(buffer,&desplazamiento, 16,registros_>tamanio_16);
		pcb->registros->tamanio_16 = registros->tamanio_16;
		*/

		while(desplazamiento < size)                                           //recivo todos los registros y las instrucciones y los meto en una lista de strings
		{
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			char* valor = malloc(tamanio);
			memcpy(valor, buffer+desplazamiento, tamanio); //rompe aca
			desplazamiento+=tamanio;
			list_add(valores, valor);
        }


		t_list* lista_registros = list_take_and_remove(valores,12);     // saco los primeros 12 elementos de la lista de strings que serian los 12 registros y los agrego en una nueva lista. En la anterior lista solo queadn los strings de las instrucciones.
		actualizar_registros_cpu(pcb,lista_registros);                          // actualizo los registros de la cpu
		instrucciones = listaAInstrucciones(valores);              // paso de lista de strings a lista de instrucciones
	    pcb->instrucciones = instrucciones;                             // actualizo lista de instrucciones en el pcb

	    return pcb;
}


t_pcb* inicializar_pcb(){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->instrucciones = list_create();
	pcb->registros = inicializarRegistros();
	return pcb;
}

void recibir_variable(int* variable, t_buffer*buffer,int* desplazamiento) {

	memcpy(&variable, buffer + (*desplazamiento), sizeof(int));
	(*desplazamiento)+=sizeof(int);
}

void actualizar_registros_cpu(t_pcb *pcb, t_list* lista_registros) {
	/*int cant = list_size(lista_registros);
	for(int i = 0;i<cant;i++) {
		char aux[4];
		char* auxp;
		auxp = list_get(lista_registros,i);
		strncpy(aux, (char *)list_get(lista_registros,i),4);
		log_info(logger, "elemento: %s \n", auxp);
		log_info(logger, "tamanio: %d \n", sizeof(*aux));
		log_info(logger, "tamanio: %d \n", sizeof(aux));
	}*/
	strcpy(registros->AX, (char *) list_get(lista_registros,0));
	strcpy(registros->BX , (char *) list_get(lista_registros,1));
	strcpy(registros->CX, list_get(lista_registros,2));
	strcpy(registros->DX, list_get(lista_registros,3));
	strcpy(registros->EAX, list_get(lista_registros,4));
	strcpy(registros->EBX, list_get(lista_registros,5));
	strcpy(registros->ECX, list_get(lista_registros,6));
	strcpy(registros->EDX, list_get(lista_registros,7));
	strcpy(registros->RAX,list_get(lista_registros,8));
	strcpy(registros->RBX, list_get(lista_registros,9));
	strcpy(registros->RCX, list_get(lista_registros,10));
	strcpy(registros->RDX, list_get(lista_registros,11));
}


void actualizar_registros_pcb(t_pcb *pcb) {

	strcpy(pcb->registros->AX,registros->AX);
	strcpy(pcb->registros->BX, registros->BX);
	strcpy(pcb->registros->CX, registros->CX);
	strcpy(pcb->registros->DX, registros->DX);
	strcpy(pcb->registros->EAX, registros->EAX);
	strcpy(pcb->registros->EBX, registros->EBX);
	strcpy(pcb->registros->ECX, registros->ECX);
	strcpy(pcb->registros->EDX, registros->EDX);
	strcpy(pcb->registros->RAX, registros->RAX);
	strcpy(pcb->registros->RBX, registros->RBX);
	strcpy(pcb->registros->RCX, registros->RCX);
	strcpy(pcb->registros->RDX, registros->RDX);
}

/* alternativa con vectores segunel tamaño
void recibir_registros(t_buffer*buffer, int* desplazamiento, int tamanio_registro, char**registro) {

	for(int i = 0; i<4;i++) {
		char valor[tamanio_registro];
		memcpy(valor, buffer + (*desplazamiento), tamanio_registro);
		(*desplazamiento)+=tamanio_registro;
		registro[4][i] = string_duplicate(valor);
	}

}
*/


void realizar_ciclo_instruccion(t_pcb * pcb){
	estado_ejec estado = CONTINUAR;
	//t_segmento* direc_fisica;

	while (estado == CONTINUAR){ // Solo continua el ciclo con SET, MOV_IN y MOV_OUT (si no hay un error)
		t_instruccion* instruccion_ejecutar = fetch(pcb->instrucciones, pcb->pc); // busco la instruccion que apunta el pc
		if(decode(instruccion_ejecutar->instruccion)){ //decode lo usamos para sabEr si la instrccion requiere memoria y para hacer el RETARDO de SET
			//int direc_logica = direc_logica(instruccion_ejecutar);
			//direc_fisica = traducir_direcciones(direc_logica);
		}
		estado = execute(instruccion_ejecutar,pcb); //execute devuelve el estado de ejecucion de la instruccion
		pcb->pc++;
	}
}

t_instruccion* fetch(t_list* instrucciones, uint32_t pc){
	log_info(logger, "Entro en fetch");
	return list_get(instrucciones,pc);
}

int decode(char* instruccion) {
	log_info(logger, "Entro en decode");
	if(strcmp(instruccion, "SET") == 0) {
		int espera = config_get_int_value(config,"RETARDO_INSTRUCCION");
		usleep(espera * 1000); // Recibe microsegundo, * 1000 -> pasamos a milisegundos
	}
	 return requiere_memoria(instruccion);
}

int requiere_memoria(char* instruccion) {

	if (strcmp(instruccion,"MOV_IN") == 0 ||
		strcmp(instruccion,"MOV_OUT")== 0 ||
		strcmp(instruccion,"F_READ") == 0||
		strcmp(instruccion,"MOV_IN")== 0 )
			           return 1;
     return 0;
}

estado_ejec execute(t_instruccion* instruccion_ejecutar,t_pcb* pcb){

	switch (id(instruccion_ejecutar->instruccion)) {
		case SET:
			 log_info(logger, "PID: %d - Ejecutando: %s - %s %s", pcb->pid, instruccion_ejecutar->instruccion, instruccion_ejecutar->parametros[0], instruccion_ejecutar->parametros[1]);
			 estado_ejec estado_set = ejecutar_set(instruccion_ejecutar->parametros[0], instruccion_ejecutar->parametros[1]);
			 if(estado_set == ERROR)
				 pcb->motivo = EXT;
			 return estado_set;

		case YIELD:
			log_info(logger,"PID: %d - Ejecutando: %s - ", pcb->pid, instruccion_ejecutar-> instruccion);
			pcb->motivo = YIELD;
			return FIN;

		case EXT:
			log_info(logger,"PID: %d - Ejecutando: %s - ", pcb->pid, instruccion_ejecutar-> instruccion);
			pcb->motivo = EXT;
			return FIN;

		case IO:
			log_info(logger,"PID: %d - Ejecutando: %s - ", pcb->pid, instruccion_ejecutar-> instruccion);
			pcb->motivo = IO;
			//TODO ver si tengo que tener el parametro de la instruccion en el pcb que envio a kernel (que indica el tiempo de bloqueo) o lo podemos buscar directo desde el kernel haciendo un listget pc-1  de la lista de instrucciones (obtengo la instruccion anterior y de ahi saco los parametros)
			return FIN;

		//TODO ver lo mismo del parametro que pasa en IO para WAIT Y SIGNAL (en este caso el parametro es un recurso)
		case WAIT:
			log_info(logger,"PID: %d - Ejecutando: %s - ", pcb->pid, instruccion_ejecutar-> instruccion);
			pcb->motivo = WAIT;
			return FIN;

		case SIGNAL:
			log_info(logger,"PID: %d - Ejecutando: %s - ", pcb->pid, instruccion_ejecutar-> instruccion);
		    pcb->motivo = SIGNAL;
			return FIN;

		default:
			log_error(logger,"Hubo un error en el ciclo de instruccion");
			return FIN;
	}

	return CONTINUAR;
}

estado_ejec ejecutar_set(char* registro, char* valor) {

	int tamanio = strlen(valor);

	if(tamanio == 4) {

		switch (registro[0]) {
		      case 'A':  strcpy(registros->AX, valor);
		                 break;
		      case 'B':  strcpy(registros->BX, valor);
		                 break;
		      case 'C':  strcpy(registros->CX, valor);
		                 break;
		      case 'D':  strcpy(registros->DX, valor);
		                 break;
		      default :  log_error(logger,"Error al ejecutar SET: el tamanio del valor a asignar es de 4 bytes pero el registro no es de dicho tamanio");		                 return FIN;
                         return ERROR;
		}
	}
	else if (tamanio == 8) {

		switch (registro[1]) {
			 case 'A':  strcpy(registros->EAX, valor);
			            break;
			 case 'B':  strcpy(registros->EBX, valor);
			            break;
			 case 'C':  strcpy(registros->ECX, valor);
			            break;
			 case 'D':  strcpy(registros->EDX, valor);
			            break;
			 default :  log_error(logger,"Error al ejecutar SET: el tamanio del valor a asignar es de 8 bytes pero el registro no es de dicho tamanio");
			            return ERROR;
		}
	}
	else if (tamanio == 16) {

		switch (registro[1]) {
			 case 'A':  strcpy(registros->RAX, valor);
			            break;
			 case 'B':  strcpy(registros->RBX, valor);
			            break;
		     case 'C':  strcpy(registros->RCX, valor);
		                break;
		     case 'D':  strcpy(registros->RDX, valor);
		                break;
		     default :  log_error(logger,"Error al ejecutar SET: el tamanio del valor a asignar es de 16 bytes pero el registro no es de dicho tamanio");
		                return ERROR;
		}
	}
   else {
		log_error(logger,"Error al ejecutar SET: el valor a asignar no es de 4/8/16 bytes");
		return ERROR;
  }

	return CONTINUAR;
}



void enviar_pcb(t_pcb* proceso, int conexion){

	char* operacion;
	t_paquete *paquete = crear_paquete(PROCESO);
	int op_tamanio = 0;
	t_instruccion* inst = malloc(sizeof(t_instruccion));
	int cant = list_size(proceso->instrucciones);
	int cant_parametros = 0;

	agregar_valor_estatico(paquete, &(proceso -> pid));
	agregar_valor_estatico(paquete, &(proceso -> pc));
	agregar_valor_estatico(paquete, &(proceso -> motivo));
	agregar_a_paquete(paquete, proceso -> registros->AX, 4);
	agregar_a_paquete(paquete, proceso -> registros->BX, 4);
	agregar_a_paquete(paquete, proceso -> registros->CX, 4);
	agregar_a_paquete(paquete, proceso -> registros->DX, 4);
	agregar_a_paquete(paquete, proceso -> registros->EAX, 8);
	agregar_a_paquete(paquete, proceso -> registros->EBX, 8);
	agregar_a_paquete(paquete, proceso -> registros->ECX, 8);
	agregar_a_paquete(paquete, proceso -> registros->EDX, 8);
	agregar_a_paquete(paquete, proceso -> registros->RAX, 16);
	agregar_a_paquete(paquete, proceso -> registros->RBX, 16);
	agregar_a_paquete(paquete, proceso -> registros->RCX, 16);
	agregar_a_paquete(paquete, proceso -> registros->RDX, 16);

	//TODO: tabla de segmentos

	for(int i = 0;i<cant;i++) {

	    inst = list_get(proceso -> instrucciones,i);
	    operacion = string_duplicate(inst -> instruccion);
	    op_tamanio = strlen(operacion)+1;

	    agregar_a_paquete(paquete,operacion,op_tamanio);

	    cant_parametros = cantParametros(operacion);
	    for(int i=0; i<cant_parametros; i++) {
	    	agregar_a_paquete(paquete,inst->parametros[i],strlen(inst->parametros[i])+1);
	    }

	}
	enviar_paquete(paquete,conexion);                // serializa el paquete y lo envia

	eliminar_paquete(paquete);                //elimina el paquete y lo que contiene

}
