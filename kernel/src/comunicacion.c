#include <comunicacion.h>
#include <utilsKernel.h>

extern t_config* config;
extern t_log* logger;
extern t_socket conexionCPU;
extern t_socket conexionMemoria;

extern pthread_mutex_t mutex_procesos_new;
extern t_list* procesosNew;
extern sem_t sem_new_a_ready, sem_finalizar;


uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

extern t_list* listaProcesosGlobal;
extern bool seguir_ejecucion;

t_socket socket_cliente;

int escucharConsolas(){
	t_list* lista;
	char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");
	t_socket server_fd = iniciar_servidor(puerto, logger);
	//printf("\nSocket conexion:%d \n",server_fd);
	free(puerto);

	while(seguir_ejecucion){
	socket_cliente = esperar_cliente(server_fd, logger); //Hace el accept
    //printf("\nsocket cliente:%d \n",socket_cliente);
	int cod_op = recibir_operacion(socket_cliente);
		if(seguir_ejecucion==0){
			log_debug(logger, "Salgo del hilo consolas");
			sem_post(&sem_finalizar);
		}else{
			if(socket_cliente != -1){
					switch (cod_op) {
							case PROGRAMA:
								t_list* buffer = recibir_paquete(socket_cliente);
								lista = listaAInstrucciones(buffer);
								list_destroy_and_destroy_elements(buffer, free);
								//send(socket_cliente, &RESULT_OK, sizeof(int), NULL);

								log_info(logger, "Me llego un paquete\n");

								t_pcb* pcb = crearPCB(lista, socket_cliente);
								list_add(listaProcesosGlobal, pcb);
								t_segmento* segmento_0 = list_get(pcb->tablaSegmentos,0);
								log_debug(logger, "El segmento 0: Id: %d | Base: %d| Limite: %d",segmento_0->id, segmento_0->base, segmento_0->limite);
								list_destroy(lista);
								pthread_mutex_lock(&mutex_procesos_new);
								list_add(procesosNew, pcb);
								pthread_mutex_unlock(&mutex_procesos_new);

								log_info(logger, "Se crea el proceso %d en NEW", pcb->pid);
								sem_post(&sem_new_a_ready);

								break;
							case -1:
								send(socket_cliente, (void *)(intptr_t)RESULT_ERROR, sizeof(uint32_t), (intptr_t)NULL);
								log_error(logger, "el cliente se desconecto. Terminando servidor");
								//return EXIT_FAILURE;
								break;

							default:
								log_warning(logger,"Operacion desconocida. No quieras meter la pata");
								break;
							}
			}
		}
	}
		log_debug(logger, "Sali while de escucharConsolas");
	return 0;
}


void mandar_pcb_a_CPU(t_pcb* proceso){
	char* operacion;
	t_paquete *paquete = crear_paquete(PROCESO);
	int op_tamanio = 0;
	t_segmento* segmento;

	int cant = list_size(proceso->instrucciones);
	int cant_parametros = 0;
	//log_info(logger, "PID:%d\n",proceso->pid);
	agregar_valor_estatico(paquete, &(proceso -> pid));
	agregar_valor_estatico(paquete, &(proceso -> pc));

	int cantidad = list_size(proceso->tablaSegmentos);
		agregar_valor_estatico(paquete,&cantidad);
		for (int i = 0; i<cantidad; i++){
			segmento = list_get(proceso->tablaSegmentos, i);
			agregar_valor_estatico(paquete,&(segmento->id));
			agregar_valor_uint(paquete,&(segmento->base));
			agregar_valor_uint(paquete,&(segmento->limite));
		}

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


	t_instruccion* inst;
	for(int i = 0;i<cant;i++) {
		//printf("Instruccion: %s\n", ((t_instruccion*)list_get(proceso -> instrucciones,i))->instruccion);
	    inst = list_get(proceso -> instrucciones,i);
	    //printf("Instruccion: %s\n",inst->instruccion);
	    operacion = copiar(inst->instruccion);
		//operacion = ((t_instruccion)list_get(proceso -> instrucciones,i))->instruccion;
	    op_tamanio = strlen(operacion)+1;

	    agregar_a_paquete(paquete,operacion,op_tamanio);

	    cant_parametros = cantParametros(operacion);
	    free(operacion);
	    for(int i=0; i<cant_parametros; i++) {
	    	agregar_a_paquete(paquete,inst->parametros[i],strlen(inst->parametros[i])+1);
	    }
	    //free(inst);
	}

	//proceso->tiempoCPU = iniciarTiempo();
	temporal_resume(proceso->tiempoCPU);
	//iniciarTiempoCPU(proceso);
	enviar_paquete(paquete,conexionCPU);                // serializa el paquete y lo envia

	eliminar_paquete(paquete);                //elimina el paquete y lo que contiene

}

void enviarAMemoria(int id_segmento, int tamanio_segmento){
	t_paquete* paquete = crear_paquete(CREATE_SEGMENT_OP);
	agregar_valor_estatico(paquete,&tamanio_segmento);
	agregar_valor_estatico(paquete,&id_segmento);

	enviar_paquete(paquete, conexionMemoria);
	eliminar_paquete(paquete);

}

void avisar_fin_a_memoria(int pid){
	t_paquete* paquete = crear_paquete(FIN_PROCESO);
	agregar_valor_estatico(paquete,&pid);
	enviar_paquete(paquete, conexionMemoria);
	eliminar_paquete(paquete);
}

void avisar_fin_a_consola(t_socket socket_consola){
	log_debug(logger, "El socket de la consola es:%d", socket_consola);
	send(socket_consola, &RESULT_OK, sizeof(int), 0);
}

/*void finalizar_proceso(t_pcb* proceso){
	avisar_fin_a_memoria(proceso->pid);
	avisar_fin_a_consola(proceso->socket_consola);
	liberar_pcb(proceso);
	sem_post(&sem_grado_multiprogramacion);
}*/

void pedirTablaSegmentos(int pid){
	t_paquete* paquete = crear_paquete(TABLA_SEGMENTOS);
	agregar_valor_estatico(paquete,&pid);
	enviar_paquete(paquete, conexionMemoria);
	eliminar_paquete(paquete);
}
