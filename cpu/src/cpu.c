#include <cpu.h>

t_log* logger;
t_config* config;
t_registros* registros;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;


int main(int argc, char* argv[]) {


	logger = iniciar_logger("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);

    t_socket conexionMemoria = iniciarConexion(config, logger, "IP_MEMORIA","PUERTO_MEMORIA");

	t_socket server_fd = iniciarServidor(config,logger, "PUERTO_ESCUCHA");

	registros = inicializarRegistros();

	t_socket socket_cliente = esperar_cliente(server_fd, logger);
	while(1){
		if(socket_cliente != -1){
			t_pcb* pcb = inicializar_pcb();
			int cod_op = recibir_operacion(socket_cliente);
			if(cod_op == PROCESO) {
				pcb = recibir_proceso(socket_cliente);
				t_instruccion* inst_privilegiada = realizar_ciclo_instruccion(pcb);
				log_info(logger, "Enviando pcb a kernel");
				actualizar_registros_pcb(pcb);
				enviar_contexto(pcb,inst_privilegiada,socket_cliente);

			} else {
				send(socket_cliente, (void *)(intptr_t)RESULT_ERROR, sizeof(uint32_t), (intptr_t)NULL);
				log_error(logger,"No me llego un proceso");
				exit(1);
			}
		}
	}
}


t_instruccion* realizar_ciclo_instruccion(t_pcb * pcb){
	estado_ejec estado = CONTINUAR;
	t_instruccion* instruccion_ejecutar;
	//t_segmento* direc_fisica;

	while (estado == CONTINUAR){ // Solo continua el ciclo con SET, MOV_IN y MOV_OUT (si no hay un error)
		instruccion_ejecutar = fetch(pcb->instrucciones, pcb->pc); // busco la instruccion que apunta el pc
		if(decode(instruccion_ejecutar->instruccion)){ //decode lo usamos para sabEr si la instrccion requiere memoria y para hacer el RETARDO de SET
			//int direc_logica = direc_logica(instruccion_ejecutar);
			//direc_fisica = traducir_direcciones(direc_logica);
		}
		estado = execute(instruccion_ejecutar,pcb); //execute devuelve el estado de ejecucion de la instruccion
		pcb->pc++;
	}
	return instruccion_ejecutar;
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
			log_info(logger,"PID: %d - Ejecutando: %s - %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar->parametros[0]);
			pcb->motivo = IO;
			//TODO ver si tengo que tener el parametro de la instruccion en el pcb que envio a kernel (que indica el tiempo de bloqueo) o lo podemos buscar directo desde el kernel haciendo un listget pc-1  de la lista de instrucciones (obtengo la instruccion anterior y de ahi saco los parametros)
			return FIN;

		//TODO ver lo mismo del parametro que pasa en IO para WAIT Y SIGNAL (en este caso el parametro es un recurso)
		case WAIT:
			log_info(logger,"PID: %d - Ejecutando: %s - %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar-> parametros[0]);
			pcb->motivo = WAIT;
			return FIN;

		case SIGNAL:
			log_info(logger,"PID: %d - Ejecutando: %s - %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar-> parametros[0]);
		    pcb->motivo = SIGNAL;
			return FIN;

		default:
			log_error(logger,"Error en execute. La CPU no conoce todavia la operacion: %s ",instruccion_ejecutar-> instruccion);
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

