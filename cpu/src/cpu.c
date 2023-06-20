#include <cpu.h>

t_log* logger;
t_config* config;
t_registros* registros;
t_socket conexionMemoria;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;


int main(int argc, char* argv[]) {


	logger = iniciar_logger("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);

    conexionMemoria = iniciarConexion(config, logger, "IP_MEMORIA","PUERTO_MEMORIA");

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
				log_error(logger,"Se cerró la conexión");
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
		decode(instruccion_ejecutar->instruccion);
		estado = execute(instruccion_ejecutar,pcb); //execute devuelve el estado de ejecucion de la instruccion
		pcb->pc++;
	}
	return instruccion_ejecutar;
}

t_instruccion* fetch(t_list* instrucciones, int pc){
	log_info(logger, "Entro en fetch");
	return list_get(instrucciones,pc);
}

void decode(char* instruccion) {
	log_info(logger, "Entro en decode");
	if(strcmp(instruccion, "SET") == 0) {
		int espera = config_get_int_value(config,"RETARDO_INSTRUCCION");
		usleep(espera * 1000); // Recibe microsegundo, * 1000 -> pasamos a milisegundos
	}
}

/*int requiere_memoria(char* instruccion) {

	if (strcmp(instruccion,"MOV_IN") == 0 ||
		strcmp(instruccion,"MOV_OUT")== 0 ||
		strcmp(instruccion,"F_READ") == 0||
		strcmp(instruccion,"F_WRITE")== 0 )
			           return 1;
     return 0;
}*/

estado_ejec execute(t_instruccion* instruccion_ejecutar,t_pcb* pcb){

	switch (id(instruccion_ejecutar->instruccion)) {
		case SET:
			 log_info(logger, "PID: %d - Ejecutando: %s - %s %s", pcb->pid, instruccion_ejecutar->instruccion, instruccion_ejecutar->parametros[0], instruccion_ejecutar->parametros[1]);
			 estado_ejec estado_set = set_registro(instruccion_ejecutar->parametros[0], instruccion_ejecutar->parametros[1]);
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
			return FIN;

		case WAIT:
			log_info(logger,"PID: %d - Ejecutando: %s - %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar-> parametros[0]);
			pcb->motivo = WAIT;
			return FIN;

		case SIGNAL:
			log_info(logger,"PID: %d - Ejecutando: %s - %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar-> parametros[0]);
		    pcb->motivo = SIGNAL;
			return FIN;

		case CREATE_SEGMENT:
			log_info(logger,"PID: %d - Ejecutando: %s - %s %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar-> parametros[0], instruccion_ejecutar-> parametros[1]);
			pcb->motivo = CREATE_SEGMENT;
		    return FIN;

		case DELETE_SEGMENT:
			log_info(logger,"PID: %d - Ejecutando: %s - %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar-> parametros[0]);
			pcb->motivo = DELETE_SEGMENT;
			return FIN;

		case MOV_IN:
			log_info(logger,"PID: %d - Ejecutando: %s - %s %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar-> parametros[0],instruccion_ejecutar-> parametros[1]);
			estado_ejec estado_mov_in = ejecutar_mov_in(pcb->pid, instruccion_ejecutar->parametros[0],instruccion_ejecutar->parametros[1], pcb->tablaSegmentos);
			if(estado_mov_in == ERROR)
				pcb->motivo = EXT;
			return estado_mov_in;

		case MOV_OUT:
		    log_info(logger,"PID: %d - Ejecutando: %s - %s %s", pcb->pid, instruccion_ejecutar-> instruccion,instruccion_ejecutar-> parametros[0], instruccion_ejecutar-> parametros[1]);
		    estado_ejec estado_mov_out = ejecutar_mov_out(pcb->pid, instruccion_ejecutar->parametros[0],instruccion_ejecutar->parametros[1], pcb->tablaSegmentos);
		    if(estado_mov_out == ERROR)
		    	pcb->motivo = EXT;
			return estado_mov_out;

		default:
			log_error(logger,"Error en execute. La CPU no conoce todavia la operacion: %s ",instruccion_ejecutar-> instruccion);
			return FIN;
	}

	return CONTINUAR;
}

estado_ejec set_registro(char* registro, char* valor) {

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
		      default :  log_error(logger,"Error al setear registro: el tamanio del valor a asignar es de 4 bytes pero el registro no es de dicho tamanio");
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
			 default :  log_error(logger,"Error al setear registro: el tamanio del valor a asignar es de 8 bytes pero el registro no es de dicho tamanio");
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
		     default :  log_error(logger,"Error al setear registro: el tamanio del valor a asignar es de 16 bytes pero el registro no es de dicho tamanio");
		                return ERROR;
		}
	}
   else {
		log_error(logger,"Error setear registro: el valor a asignar no es de 4/8/16 bytes");
		return ERROR;
  }

	return CONTINUAR;
}


char* get_registro(char*registro) {

	char* valor;

		if (registro[1]== 'E') {

			valor = malloc(8*sizeof(char));

			switch (registro[1]) {
				 case 'A':  strcpy(valor, registros->EAX);
				            break;
				 case 'B':  strcpy(valor, registros->EBX);
				            break;
				 case 'C':  strcpy(valor, registros->ECX);
				            break;
				 case 'D':  strcpy(valor, registros->EDX);
				            break;
			}
		}
		else if (registro[1] == 'R') {

			valor = malloc(16*sizeof(char));
			switch (registro[1]) {
			        case 'A':  strcpy(valor, registros->RAX);
					           break;
		            case 'B':  strcpy(valor, registros->RBX);
						       break;
			        case 'C':  strcpy(valor, registros->RCX);
							   break;
				    case 'D':  strcpy(valor, registros->RDX);
							   break;
			}
		}
		else {
			valor = malloc(4*sizeof(char));
			switch (registro[0]) {
			     case 'A':  strcpy(valor, registros->AX);
			                break;
			     case 'B':  strcpy(valor, registros->BX);
						    break;
                 case 'C':  strcpy(valor, registros->CX);
						    break;
			     case 'D':  strcpy(valor, registros->DX);
						    break;
				}
		}

	return valor;

}


estado_ejec ejecutar_mov_in(int pid, char* registro, char* direc, tabla_segmentos tabla_de_segmentos) {

direc_logica* direcLogica = crear_direc_logica(direc);
int tamanio_a_leer = tamanio_registro(registro);
estado_ejec resultado = ejecutar_mov(pid, tamanio_a_leer,direcLogica,tabla_de_segmentos);

if(resultado == CONTINUAR) {

	int direc_fisica = obtener_direc_fisica(direcLogica,tabla_de_segmentos);
	char* valor = leer_memoria(direc_fisica,tamanio_a_leer);
	set_registro(registro,valor);
	log_info(logger, "PID: %d - Acción: LEER - Segmento: %d - Dirección Física: %d - Valor: %s", pid, direcLogica->numero_segmento, direc_fisica, valor);
	free(valor);
}

return resultado;
}

estado_ejec ejecutar_mov_out(int pid, char* direc,char* registro, tabla_segmentos tabla_de_segmentos) {

direc_logica* direcLogica = crear_direc_logica(direc);
int tamanio_a_escribir = tamanio_registro(registro);
estado_ejec resultado = ejecutar_mov(pid,tamanio_a_escribir, direcLogica, tabla_de_segmentos);


if(resultado == CONTINUAR) {
	 int direc_fisica = obtener_direc_fisica(direcLogica,tabla_de_segmentos);
	 char* valor = get_registro(registro);
	 escribir_memoria(direc_fisica,valor, tamanio_a_escribir);
	 log_info(logger, "PID: %d - Acción: ESCRIBIR - Segmento: %d - Dirección Física: %d - Valor: %s", pid, direcLogica->numero_segmento, direc_fisica, valor);
	 free(valor);
}
return resultado;

}


estado_ejec ejecutar_mov(int pid, int tamanio_valor, direc_logica* direcLogica, tabla_segmentos tabla_de_segmentos) {

		if (verificar_num_segmento(direcLogica->numero_segmento,tabla_de_segmentos))
			{
			 t_segmento*segmento = list_get(tabla_de_segmentos,direcLogica->numero_segmento);
			 if (no_produce_seg_fault(pid, direcLogica->desplazamiento,tamanio_valor, segmento) == 1)
				 return CONTINUAR;
			}
		return ERROR;
}


direc_logica* crear_direc_logica(char* direc) {

	direc_logica* direcLogica = malloc(sizeof(direc_logica));
	int direc_numero = atoi(direc);
	int tam_max_segmento = config_get_int_value(config,"TAM_MAX_SEGMENTO");
	direcLogica->numero_segmento = floor(direc_numero/tam_max_segmento);
	direcLogica->desplazamiento = direc_numero % tam_max_segmento;

	return direcLogica;
}


int verificar_num_segmento(int num_segmento,tabla_segmentos tabla_de_segmentos) {

	int cantidad_segmentos = list_size(tabla_de_segmentos);     //cuenta tambien el segmento 0

	if (num_segmento < cantidad_segmentos)
		return 1;

	return 0;
}

int no_produce_seg_fault(int pid, int desplazamiento,int tamanio_a_leer, t_segmento*segmento) {

	u_int32_t tamanio = segmento->limite - segmento->base;
	if ((desplazamiento + tamanio_a_leer) > tamanio)
	{
		log_info(logger, "PID: %d - Error SEG_FAULT- Segmento: %d  - Offset: %d - Tamaño: %d"
,pid,segmento->id, desplazamiento, tamanio);
		return 0;
	}

	return 1;
}

int tamanio_registro(char*registro) {

	switch(registro[0]) {
	case 'R': return 16;
	case 'E': return 8;
	default: return 4;
	}
}


u_int32_t obtener_direc_fisica(direc_logica* direcLogica,tabla_segmentos tabla_de_segmentos){

	t_segmento*segmento = list_get(tabla_de_segmentos,direcLogica->numero_segmento);
	int direc_fisica = segmento->base + direcLogica->desplazamiento;

	return direc_fisica;
}






