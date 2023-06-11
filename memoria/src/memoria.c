#include <memoria.h>

t_log* logger;
t_config* config;
void* espacioMemoria;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;
extern tabla_segmentos* tablaSegmentos;

int main(int argc, char* argv[]) {
	logger = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);

	t_socket server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");

	t_socket socket_CPU;
	socket_CPU = esperar_cliente(server_fd, logger); //Hace el accept

	t_socket socket_KERNEL;
	socket_CPU = esperar_cliente(server_fd, logger); //Hace el accept

	t_socket socket_FS;
	socket_CPU = esperar_cliente(server_fd, logger); //Hace el accept

	enviarSegmentosKernel();

	while(1){
		//t_socket socket_cliente = esperar_cliente(server_fd, logger);
		if(socket_cliente != -1){
			inicializarEstructuras();
			int cod_op = recibir_operacion(socket_cliente);
			switch (cod_op) {
			case CREATE_SEGMENT:
				//list_add(nuevoSegmento, segmento);
				break;
			case DELETE_SEGMENT:
				break;
			default:
				send(socket_cliente, (void *)(intptr_t)RESULT_ERROR, sizeof(uint32_t), (intptr_t)NULL);
				log_error(logger,"Se cerró la conexión");
				exit(1);
			}
		}
	}
}

