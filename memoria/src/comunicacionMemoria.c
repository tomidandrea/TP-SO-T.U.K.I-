#include "comunicacionMemoria.h"

extern t_socket server_fd;
extern t_log* logger;
extern t_config* config;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;
extern sem_t sem_cpu, sem_kernel;
extern t_dictionary* diccionarioTablas;
extern t_segmento* segmento0;
/*ACLARACION: memoria tiene que hacer todos los accept(en esperar_cliente())
 * para ponerse a ejecutar, porque se queda bloqueado hasta que se conecten
 * todos los modulos primero
 *
 * Orden de ejecución: Memoria, CPU, Kernel
 * por ahora le saque el esperar al FS
 */

void escucharKernel(){
	log_debug(logger, "Entro hilo para escuchar kernel");
	log_debug(logger, "socket memoria: %d", server_fd);
	sem_wait(&sem_cpu);
	t_socket socket_kernel = esperar_cliente(server_fd, logger);
	log_debug(logger, "socket kernel: %d", socket_kernel);
	//inicializarEstructuras();
	//TODO jarwi agus:kernel no está esperando todavia que le manden nada
	/*aca tendrian que hacer un recv (para que memoria se bloquee)
	 * y en kernel un send para pedir la tabla de segmentos
	 * luego kernel haga un recv y memoria el send
	 * (capaz lo tienen que meter en el while)
	*/
	//
	log_info(logger, "Inicializando estructuras...");
	inicializarEstructuras();

	while(1){
		if(socket_kernel != -1){
			int cod_op = recibir_operacion(socket_kernel);
			switch (cod_op) {
			case TABLA_SEGMENTOS:
				tabla_segmentos tablaSegmentos = list_create();
				int pid_int = recibirPID(socket_kernel);
				char* pid = string_itoa(pid_int);
				list_add(tablaSegmentos, segmento0);

				dictionary_put(diccionarioTablas, pid, tablaSegmentos);

				log_info(logger, "Enviando tabla de segmentos...");
				enviarSegmentosKernel(socket_kernel, tablaSegmentos);
				break;
			case CREATE_SEGMENT_OP:
				//t_pedido_segmento* pedido = recibirCrearSegmento(socket_kernel);
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
