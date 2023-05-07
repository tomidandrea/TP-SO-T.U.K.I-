#include <cpu.h>

t_log* logger;
t_config* config;
extern t_registros* registros;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

int main(int argc, char* argv[]) {

	t_list* lista = list_create(); //ver si es necesaria
	logger = iniciar_logger("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);
	char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

	registros = inicializarRegistros();

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
				log_info(logger, "Enviando pcb a kernel");
				// TODO enviar_pcb(pcb,estado);

			} else {
				send(socket_cliente, (void *)RESULT_ERROR, sizeof(int), NULL);
				log_error(logger,"No me llego un proceso");

			}
		}
	}
}

void recibir_variable(int* variable, t_buffer*buffer,int* desplazamiento) {

	memcpy(&variable, buffer + (*desplazamiento), sizeof(int));
	(*desplazamiento)+=sizeof(int);
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


void enviar_pcb(t_pcb* proceso, int conexion){

	char* operacion;
	t_paquete *paquete = crear_paquete(PROCESO);
	int op_tamanio = 0;
	t_instruccion* inst = malloc(sizeof(t_instruccion));
	int cant = list_size(proceso->instrucciones);
	int cant_parametros = 0;

	agregar_valor_estatico(paquete, &(proceso -> pid));
	agregar_valor_estatico(paquete, &(proceso -> pc));
	agregar_valor_estatico(paquete, &(proceso->estado_ejec));
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

