#include "comunicacionMemoria.h"

extern t_socket server_fd;
extern t_log* logger;
extern t_config* config;
extern sem_t sem_cpu, sem_kernel;
extern t_dictionary* diccionarioTablas;
extern t_segmento* segmento0;

uint32_t RESULT_ERROR = 1;

void escucharKernel(){
	log_debug(logger, "Entro hilo para escuchar kernel");
	log_debug(logger, "socket memoria: %d", server_fd);
	sem_wait(&sem_cpu);
	t_socket socket_kernel = esperar_cliente(server_fd, logger);
	log_debug(logger, "socket kernel: %d", socket_kernel);

	while(1){
		if(socket_kernel != -1){
			int cod_op = recibir_operacion(socket_kernel);
			switch (cod_op) {
			case TABLA_SEGMENTOS:
				tabla_segmentos tablaSegmentos = list_create();
				list_add(tablaSegmentos, segmento0);

				int pid_int = recibirPID(socket_kernel);
				char* pid = string_itoa(pid_int);
				dictionary_put(diccionarioTablas, pid, tablaSegmentos);

				log_info(logger, "Enviando tabla de segmentos de proceso %d", pid_int);
				enviarSegmentosKernel(socket_kernel, tablaSegmentos);
				break;
			case CREATE_SEGMENT_OP:
				t_pedido_segmento* pedido = recibirPedidoSegmento(socket_kernel);
				char* pid1 = string_itoa(pedido->pid);
				crearSegmento(pedido);
				tabla_segmentos tablaSegmentos1 = dictionary_get(diccionarioTablas, pid1);
				enviarSegmentosKernel(socket_kernel, tablaSegmentos1);

//				                switch (estado) {
//				                    case HAY_ESPACIO_CONTIGUO:
//				                        log_info(logger, "Creando segmento...");
//
//				                        break;
//				                    case ESPACIO_NO_CONTIGUO:
//				                        log_info(logger, "Debe compactarse el espacio");
//				                        break;
//				                    case SIN_ESPACIO:
//				                        log_info(logger, "Out of memory");
//				                        //TODO: informar a kernel
//				                        break;
//				                    default:
//				                    	log_error(logger,"Error en obtener estado memoria");
//				                    	break;
//				                }

				//todo hacer toda la vaina de verificar cosas xd chamaco
				break;
			case DELETE_SEGMENT_OP:
				//t_pedido_segmento* pedido = recibirCrearSegmento(socket_kernel);
				break;
			default:
				send(socket_kernel, (void *)(intptr_t)RESULT_ERROR, sizeof(uint32_t), (intptr_t)NULL);
				log_error(logger,"Se cerró la conexión");
				exit(1);
			}
		}else{
			log_error(logger,"Socket kernel == -1, la conexión se cerró");
		}
	}
}

void escucharCPU(){
	log_debug(logger, "Entro hilo para escuchar CPU");
	t_socket socket_cpu = esperar_cliente(server_fd, logger);
	sem_post(&sem_cpu);
	while(1){
		if(socket_cpu != -1){
			int cod_op = recibir_operacion(socket_cpu);
			// cosas q pide cpu xd
		}
	}
}

void escucharFS(){
	log_debug(logger, "Entro hilo para escuchar FS");
	t_socket socket_file_system;
	//t_socket socket_file_system = esperar_cliente(server_fd, logger);
	while(1){
		if(socket_file_system != -1){
			int cod_op = recibir_operacion(socket_file_system);
			// cosas q pide file system xd
		}
	}
}
