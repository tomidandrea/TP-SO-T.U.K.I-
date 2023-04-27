#include "cpu.h"

t_log* logger;
t_config* config;

t_registros* registros;

int main(int argc, char* argv[]) {

	logger = iniciar_logger("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);
	char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");


	t_socket server_fd = iniciar_servidor(puerto, logger);
	while(1){
		t_socket socket_cliente = esperar_cliente(server_fd, logger);
		t_pcb* pcb = inicializar_pcb();
		int cod_op = recibir_operacion(socket_cliente);
		if(cod_op == PROCESO) {
			// TODO pcb = recibir_proceso();
			estado_ejec estado = realizar_ciclo_instruccion(pcb);
			// TODO enviar_pcb(pcb,estado);
		} else {
			log_error(logger,"No me llego un proceso");
		}
	}
}

t_pcb* recibir_proceso(t_socket socket_cliente) {
	    int size;
		int desplazamiento = 0;
		void * buffer;
		t_list* valores = list_create();
		int tamanio;

		//printf("hola, recibiendo paquete\n");

		buffer = recibir_buffer(&size, socket_cliente);

		//printf("recibiendo buffer\n");
		while(desplazamiento < size)
		{
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			char* valor = malloc(tamanio);
			memcpy(valor, buffer+desplazamiento, tamanio);
			desplazamiento+=tamanio;
			list_add(valores, valor);
}


t_pcb* inicializar_pcb(){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->instrucciones = list_create();
//	pcb->registros = inicializar_registros();
	return pcb;
}


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


