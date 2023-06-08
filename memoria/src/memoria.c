#include <memoria.h>

t_log* logger;
t_config* config;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

int main(int argc, char* argv[]) {
	logger = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);

	t_socket server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");

	inicializarEstructuras(config);

	while(1){
		t_socket socket_cliente = esperar_cliente(server_fd, logger);
		if(socket_cliente != -1){
			int cod_op = recibir_operacion(socket_cliente);
			switch (cod_op) {
			case CREATE_SEGMENT:

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
