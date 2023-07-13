#include <cpu.h>

t_log* logger;
t_config* config;
t_registros* registros;
t_socket conexionMemoria;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;
// La uso solo para mandarle a kernel la direccion fisica para F_READ y F_WRITE (ver si hay otra forma)
u_int32_t direcFisicaAEnviar = 0;

int main(int argc, char* argv[]) {


	logger = iniciar_logger("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);

    conexionMemoria = iniciarConexion(config, logger, "IP_MEMORIA","PUERTO_MEMORIA");

	t_socket server_fd = iniciarServidor(config,logger, "PUERTO_ESCUCHA");

	registros = inicializarRegistros();

	t_socket socket_cliente = esperar_cliente(server_fd, logger);
	while(1){
		if(socket_cliente != -1){
			int cod_op = recibir_operacion(socket_cliente);
			if(cod_op == PROCESO) {
				t_pcb* pcb = recibir_proceso(socket_cliente);
				t_instruccion* inst_privilegiada = realizar_ciclo_instruccion(pcb);

				log_info(logger, "Enviando pcb a kernel");
				actualizar_registros_pcb(pcb);
				enviar_contexto(pcb,inst_privilegiada,socket_cliente);
				liberar_proceso(pcb);

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
		printf("------------------------------ PC: %d ------------------------------\n", pcb->pc);
		instruccion_ejecutar = fetch(pcb->instrucciones, pcb->pc); // busco la instruccion que apunta el pc
		decode(instruccion_ejecutar->instruccion);
		estado = execute(instruccion_ejecutar,pcb); //execute devuelve el estado de ejecucion de la instruccion
		printf("\nEstado de la instruccion: %d\n", estado);
		pcb->pc++;
	}
	return instruccion_ejecutar;
}

t_instruccion* fetch(t_list* instrucciones, int pc){
	//log_info(logger, "Entro en fetch");
	return list_get(instrucciones,pc);
}

void decode(char* instruccion) {
	//log_info(logger, "Entro en decode");
	if(strcmp(instruccion, "SET") == 0) {
		int espera = config_get_int_value(config,"RETARDO_INSTRUCCION");
		usleep(espera * 1000); // Recibe microsegundo, * 1000 -> pasamos a milisegundos
	}
}

estado_ejec execute(t_instruccion* instruccion_ejecutar,t_pcb* pcb){

	direc_logica* direcLogica;

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
		    	pcb->motivo = SEG_FAULT;
			return estado_mov_out;
		case F_OPEN:
				log_info(logger,"PID: %d - Ejecutando: %s - %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar->parametros[0]);
				pcb->motivo = F_OPEN;
				return FIN;
		case F_TRUNCATE:
					log_info(logger,"PID: %d - Ejecutando: %s - %s %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar->parametros[0], instruccion_ejecutar-> parametros[1]);
					pcb->motivo = F_TRUNCATE;
					return FIN;
		case F_CLOSE:
					log_info(logger,"PID: %d - Ejecutando: %s - %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar->parametros[0]);
					pcb->motivo = F_CLOSE;
					return FIN;
		case F_SEEK:
					log_info(logger,"PID: %d - Ejecutando: %s - %s %s", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar->parametros[0], instruccion_ejecutar-> parametros[1]);
					pcb->motivo = F_SEEK;
				    return FIN;
		case F_READ:
					log_info(logger,"PID: %d - Ejecutando: %s - %s %s %s", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar->parametros[0], instruccion_ejecutar-> parametros[1], instruccion_ejecutar-> parametros[2]);
					pcb->motivo = F_READ;
					direcLogica = crear_direc_logica(instruccion_ejecutar->parametros[1]);
					direcFisicaAEnviar = obtener_direc_fisica(direcLogica,pcb->tablaSegmentos);
					log_debug(logger, "La direccion fisica a enviar a kernel es: %d", direcFisicaAEnviar);
					free(direcLogica);
				    return FIN;
		case F_WRITE:
					log_info(logger,"PID: %d - Ejecutando: %s - %s %s %s ", pcb->pid, instruccion_ejecutar-> instruccion, instruccion_ejecutar->parametros[0], instruccion_ejecutar-> parametros[1], instruccion_ejecutar-> parametros[2]);
					pcb->motivo = F_WRITE;
					direcLogica = crear_direc_logica(instruccion_ejecutar->parametros[1]);
					direcFisicaAEnviar = obtener_direc_fisica(direcLogica,pcb->tablaSegmentos);
					log_debug(logger, "La direccion fisica a enviar a kernel es: %d", direcFisicaAEnviar);
					free(direcLogica);
				    return FIN;
		default:
			log_error(logger,"Error en execute. La CPU no conoce todavia la operacion: %s ",instruccion_ejecutar-> instruccion);
			return FIN;
	}

	return CONTINUAR;
}

estado_ejec set_registro(char* registro, char* valor) {

	int tamanio = strlen(valor);
	log_debug(logger, "Valor a setear: %s de tamaño %d", valor, tamanio);
	if(tamanio == 4) {

		switch (registro[0]) {
		      case 'A':  memcpy(registros->AX, valor, 4);
		      /*for (int i = 0; i < 4; ++i) {
		      	  log_debug(logger, "Valor%d  en AX: %c", i, registros->AX[i]);
			  }*/
		                 break;
		      case 'B':  memcpy(registros->BX, valor, 4);
			   			/*	for (int i = 0; i < 4; ++i) {
		                          log_debug(logger, "Valor%d  en BX: %c", i, registros->BX[i]);
		                    }*/
		                 break;
		      case 'C':  memcpy(registros->CX, valor, 4);
		                 break;
		      case 'D':  memcpy(registros->DX, valor, 4);
		                 break;
		      default :  log_error(logger,"Error al setear registro: el tamanio del valor a asignar es de 4 bytes pero el registro no es de dicho tamanio");
                         return ERROR;
		}
	}
	else if (tamanio == 8) {

		switch (registro[1]) {
			 case 'A':  memcpy(registros->EAX, valor, 8);
			            break;
			 case 'B':  memcpy(registros->EBX, valor, 8);
			            break;
			 case 'C':  memcpy(registros->ECX, valor, 8);
			            break;
			 case 'D':  memcpy(registros->EDX, valor, 8);
			            break;
			 default :  log_error(logger,"Error al setear registro: el tamanio del valor a asignar es de 8 bytes pero el registro no es de dicho tamanio");
			            return ERROR;
		}
	}
	else if (tamanio == 16) {

		switch (registro[1]) {
			 case 'A':  memcpy(registros->RAX, valor, 16);
						/*for (int i = 0; i < 16; ++i) {
								log_debug(logger, "Valor%d  en RAX: %c", i, registros->RAX[i]);
						}*/
			            break;
			 case 'B':  memcpy(registros->RBX, valor, 16);
			            break;
		     case 'C':  memcpy(registros->RCX, valor, 16);
		                break;
		     case 'D':  memcpy(registros->RDX, valor, 16);
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
	/*for (int i = 0; i < 4; ++i) {
			  log_debug(logger, "Valor%d  en AX: %c", i, registros->AX[i]);
		  }*/
		if (registro[0]== 'E') {

			valor = malloc(9*sizeof(char));

			switch (registro[1]) {
				 case 'A':  memcpy(valor, registros->EAX, 8);
				            break;
				 case 'B':  memcpy(valor, registros->EBX, 8);
				            break;
				 case 'C':  memcpy(valor, registros->ECX, 8);
				            break;
				 case 'D':  memcpy(valor, registros->EDX, 8);
				            break;
			}
			valor[8] = '\0';
		}
		else if (registro[0] == 'R') {

			valor = malloc(17*sizeof(char));
			switch (registro[1]) {
			        case 'A':  memcpy(valor, registros->RAX, 16);
					           break;
		            case 'B':  memcpy(valor, registros->RBX, 16);
						       break;
			        case 'C':  memcpy(valor, registros->RCX, 16);
							   break;
				    case 'D':  memcpy(valor, registros->RDX, 16);
							   break;
			}
			valor[16] = '\0';
		}
		else {
			valor = malloc(5*sizeof(char));
			switch (registro[0]) {
			     case 'A':  memcpy(valor, registros->AX, 4);
			                break;
			     case 'B':  memcpy(valor, registros->BX, 4);
						    break;
                 case 'C':  memcpy(valor, registros->CX, 4);
						    break;
			     case 'D':  memcpy(valor, registros->DX, 4);
						    break;
				}
			valor[4] = '\0';
		}

		log_debug(logger, "Valor del registro: %s", valor);
	return valor;
}


estado_ejec ejecutar_mov_in(int pid, char* registro, char* direc, tabla_segmentos tabla_de_segmentos) {

direc_logica* direcLogica = crear_direc_logica(direc);
int tamanio_a_leer = tamanio_registro(registro);
estado_ejec resultado = ejecutar_mov(pid, tamanio_a_leer,direcLogica,tabla_de_segmentos);

if(resultado == CONTINUAR) {
	u_int32_t direc_fisica = obtener_direc_fisica(direcLogica,tabla_de_segmentos);
	char* valor = leer_memoria(pid, direc_fisica,tamanio_a_leer); //tiene el \0
	set_registro(registro,valor);
	log_info(logger, "PID: %d - Acción: LEER - Segmento: %d - Dirección Física: %d - Valor: %s", pid, direcLogica->numero_segmento, direc_fisica, valor);
	free(valor);
}
free(direcLogica);

return resultado;
}

estado_ejec ejecutar_mov_out(int pid, char* direc,char* registro, tabla_segmentos tabla_de_segmentos) {

direc_logica* direcLogica = crear_direc_logica(direc);
int tamanio_a_escribir = tamanio_registro(registro);
estado_ejec resultado = ejecutar_mov(pid,tamanio_a_escribir, direcLogica, tabla_de_segmentos);


if(resultado == CONTINUAR) {
	 u_int32_t direc_fisica = obtener_direc_fisica(direcLogica,tabla_de_segmentos);
	 char* valor = get_registro(registro); //tiene el \0
	 char valor_registro[tamanio_a_escribir];
	 memcpy(valor_registro, valor, tamanio_a_escribir); //le saco el \0
	 escribir_memoria(pid, direc_fisica,valor, tamanio_a_escribir);
	 log_info(logger, "PID: %d - Acción: ESCRIBIR - Segmento: %d - Dirección Física: %d - Valor: %s", pid, direcLogica->numero_segmento, direc_fisica, valor);
	 free(valor);
}
free(direcLogica);
return resultado;

}


estado_ejec ejecutar_mov(int pid, int tamanio_valor, direc_logica* direcLogica, tabla_segmentos tabla_de_segmentos) {

		if (verificar_num_segmento(direcLogica->numero_segmento,tabla_de_segmentos))
			{
			 t_segmento*segmento = obtenerSegmentoPorId(tabla_de_segmentos, direcLogica->numero_segmento);
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

	for(int i = 0;i<cantidad_segmentos;i++){
		t_segmento* seg = list_get(tabla_de_segmentos, i);
		if (num_segmento == seg->id)
			return 1;
	}
	return 0;
}

int no_produce_seg_fault(int pid, int desplazamiento,int tamanio_a_leer, t_segmento*segmento) {

	u_int32_t tamanio = segmento->limite - segmento->base;
	if ((desplazamiento + tamanio_a_leer) > tamanio)
	{
		log_error(logger, "PID: %d - Error SEG_FAULT- Segmento: %d  - Offset: %d - Tamaño: %d"
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

	t_segmento*segmento = obtenerSegmentoPorId(tabla_de_segmentos,direcLogica->numero_segmento);
	int direc_fisica = segmento->base + direcLogica->desplazamiento;

	return direc_fisica;
}






