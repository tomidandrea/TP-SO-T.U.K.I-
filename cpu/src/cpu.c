#include "cpu.h"

t_log* logger;
t_config* config;
t_registros* registros;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

int main(int argc, char* argv[]) {

	t_list* lista = list_create(); //ver si es necesaria
	logger = iniciar_logger("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);
	char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");


	t_socket server_fd = iniciar_servidor(puerto, logger);
	free(puerto);

	t_socket socket_cliente = malloc(sizeof(int));
	while(1){
		t_socket socket_cliente = esperar_cliente(server_fd, logger);
		if(socket_cliente != -1){
			t_pcb* pcb = inicializar_pcb();
			int cod_op = recibir_operacion(socket_cliente);
			if(cod_op == PROCESO) {
				pcb = recibir_proceso(socket_cliente);
				estado_ejec estado = realizar_ciclo_instruccion(pcb);
				// TODO enviar_pcb(pcb,estado);
			} else {
				send(socket_cliente, (void *)RESULT_ERROR, sizeof(int), NULL);
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
		int pid;
		int pc;
		t_list* instrucciones = list_create();

		//printf("hola, recibiendo proceso\n");

		//printf("recibiendo buffer\n");

		buffer = recibir_buffer(&size, socket_cliente);

		recibir_variable(&pid,buffer,&desplazamiento);      // recibo pid
		pcb->pid = pid;                                     // actualizo pid en el pcb

		recibir_variable(&pc,buffer,&desplazamiento);       //recivo pc
		pcb->pc = pc;                                      // actualizo pc en el pcb


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
			memcpy(valor, buffer+desplazamiento, tamanio);
			desplazamiento+=tamanio;
			list_add(valores, valor);
        }


		t_list* lista_registros = list_take_and_remove(valores,12);     // saco los primeros 12 elementos de la lista de strings que serian los 12 registros y los agrego en una nueva lista. En la anterior lista solo queadn los strings de las instrucciones.
		actualizar_registros(pcb,lista_registros);                          // actualizo los registros de la cpu
		instrucciones = listaAInstrucciones(valores);              // paso de lista de strings a lista de instrucciones
	    pcb->instrucciones = instrucciones;                             // actualizo lista de instrucciones en el pcb

	    return pcb;
}


t_pcb* inicializar_pcb(){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->instrucciones = list_create();
//	pcb->registros = inicializar_registros();
	return pcb;
}

void recibir_variable(int* variable, t_buffer*buffer,int* desplazamiento) {

	memcpy(&variable, buffer + (*desplazamiento), sizeof(int));
	(*desplazamiento)+=sizeof(int);
}

void actualizar_registros(t_pcb *pcb, t_list*lista_registros) {
	strcpy(registros->AX ,list_get(lista_registros,0));
	strcpy(pcb->registros->AX,registros->AX);
	strcpy(registros->BX , list_get(lista_registros,1));
	strcpy(pcb->registros->BX, registros->BX);
	strcpy(registros->CX, list_get(lista_registros,2));
	strcpy(pcb->registros->CX, registros->CX);
	strcpy(registros->DX, list_get(lista_registros,3));
	strcpy(pcb->registros->DX, registros->DX);
	strcpy(registros->EAX, list_get(lista_registros,4));
	strcpy(pcb->registros->EAX, registros->EAX);
	strcpy(registros->EBX, list_get(lista_registros,5));
	strcpy(pcb->registros->EBX, registros->EBX);
	strcpy(registros->ECX, list_get(lista_registros,6));
	strcpy(pcb->registros->ECX, registros->ECX);
	strcpy(registros->EDX, list_get(lista_registros,7));
	strcpy(pcb->registros->EDX, registros->EDX);
	strcpy(registros->RAX,list_get(lista_registros,8));
	strcpy(pcb->registros->RAX, registros->RAX);
	strcpy(registros->RBX, list_get(lista_registros,9));
	strcpy(pcb->registros->RBX, registros->RBX);
	strcpy(registros->RCX, list_get(lista_registros,10));
	strcpy(pcb->registros->RCX, registros->RCX);
	strcpy(registros->RDX, list_get(lista_registros,11));
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


estado_ejec realizar_ciclo_instruccion(t_pcb * pcb){
	estado_ejec estado = CONTINUAR;
	//t_segmento* direc_fisica;

	while (estado == CONTINUAR){ // Solo continua el ciclo con SET, MOV_IN y MOV_OUT
		t_instruccion* instruccion_ejecutar = fetch(pcb->instrucciones, pcb->pc); // busco la instruccion que apunta el pc

		if(decode(instruccion_ejecutar->instruccion)){ //decode lo usamos para sabEr si la instrccion requiere memoria y para hacer el RETARDO de SET
			//int direc_logica = direc_logica(instruccion_ejecutar);
			//direc_fisica = traducir_direcciones(direc_logica);
		}
		estado = execute(instruccion_ejecutar,pcb->pid); //execute devuelve el estado de ejecucion de la instruccion
		pcb->pc++;
	}
	return estado;
}

t_instruccion* fetch(t_list* instrucciones, uint32_t pc){
	return list_get(instrucciones,pc);
}

int decode(char* instruccion) {
	if(strcmp(instruccion, "SET") == 0) {
		int espera = config_get_int_value(config,"RETARDO_INSTRUCCION");
		usleep(espera * 1000);
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

estado_ejec execute(t_instruccion* instruccion_ejecutar, int pid){

	switch (instruccion_ejecutar->instruccion[0]) {
		case 'S':
			 log_info(logger, "PID: %d - Ejecutando: %s - %s %s", pid, instruccion_ejecutar->instruccion, instruccion_ejecutar->parametros[0], instruccion_ejecutar->parametros[1]);
			 ejecutar_set(instruccion_ejecutar->parametros[0], instruccion_ejecutar->parametros[1]);
			 return CONTINUAR;
		case 'Y':
			log_info(logger,"PID: %d - Ejecutando: %s - ", pid, instruccion_ejecutar-> instruccion);
			return DESALOJAR;

		case 'E':
			log_info(logger,"PID: %d - Ejecutando: %s - ", pid, instruccion_ejecutar-> instruccion);
			return FIN;
		default:
			log_error(logger,"Hubo un error en el ciclo de instruccion");
			break;
	}

	return CONTINUAR;
}

void ejecutar_set(char* registro, char* valor) {

	int tamanio = strlen(valor);

	if(entra_en_registro(registro,valor)) {
	if(tamanio <= 4) {

		switch (registro[0]) {
		      case 'A':  strcpy(registros->AX, valor);
		                 break;
		      case 'B' : strcpy(registros->BX, valor);
		                 break;
		      case 'C':  strcpy(registros->CX, valor);
		                 break;
		      case 'D' : strcpy(registros->DX, valor);
		                 break;
		}
	}
	else if (tamanio <= 8) {
		switch (registro[1]) {
			 case 'A':  strcpy(registros->EAX, valor);
			            break;
			 case 'B':  strcpy(registros->EBX, valor);
			            break;
			 case 'C':  strcpy(registros->ECX, valor);
			            break;
			 case 'D':  strcpy(registros->EDX, valor);
			            break;
		}
	}
	else {
		switch (registro[1]) {
			 case 'A':  strcpy(registros->RAX, valor);
			            break;
			 case 'B':  strcpy(registros->RBX, valor);
			            break;
		     case 'C':  strcpy(registros->RCX, valor);
		                break;
		     case 'D':  strcpy(registros->RDX, valor);
		                break;
		}
	}
  }
   else {
		log_error(logger,"No es posible almacenar %s en el registro %s ya que supera el limite de capacidad", valor,registro);
  }
}



int entra_en_registro(char*registro,char*valor) {

	int tamanio_valor = strlen(valor);
	int caracteres= strlen(registro);

	if ((tamanio_valor >4 && caracteres == 2) || (tamanio_valor >8 && registro[0] != 'R') || tamanio_valor > 16 )
		return 0;
	else
		return 1;
}


