#include "comunicacionMemoria.h"

t_socket server_fd;
extern t_log* logger;
extern t_config* config;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

void escucharKernel(){
	iniciarServidor(config, logger,"PUERTO_ESCUCHA");

	t_socket socket_kernel = esperar_cliente(server_fd, logger);
	inicializarEstructuras();
	enviarSegmentosKernel(socket_kernel);
	while(1){
		if(socket_kernel != -1){
			int cod_op = recibir_operacion(socket_kernel);
			switch (cod_op) {
			case CREATE_SEGMENT:
				//list_add(nuevoSegmento, segmento);
				break;
			case DELETE_SEGMENT:
				break;
			default:
				send(socket_kernel, (void *)(intptr_t)RESULT_ERROR, sizeof(uint32_t), (intptr_t)NULL);
				log_error(logger,"Se cerró la conexión");
				exit(1);
			}
		}
	}
}

void escucharCPU(){
	t_socket socket_cpu = esperar_cliente(server_fd, logger);
	while(1){
		if(socket_cpu != -1){
			// cosas q pide cpu xd
		}
	}
}

void escucharFS(){
	t_socket socket_file_system = esperar_cliente(server_fd, logger);
	while(1){
		if(socket_file_system != -1){
			// cosas q pide file system xd
		}
	}
}
