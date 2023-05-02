#include <cpu.h>
#include <readline/readline.h>

t_log* logger;
t_config* config;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

int main(int argc, char* argv[]) {
	t_list* lista = list_create();
	logger = iniciar_logger("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);
	char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

	t_socket server_fd = iniciar_servidor(puerto, logger);
	free(puerto);

	t_socket socket_cliente = malloc(sizeof(int));
	while(1){
		t_socket socket_cliente = esperar_cliente(server_fd, logger);
		if(socket_cliente != -1){
						int cod_op = recibir_operacion(socket_cliente);
						switch (cod_op) {
								case PAQUETE:

									lista = recibir_paquete(socket_cliente);
									log_info(logger, "Me llego un paquete\n");

									break;
								case -1:
									send(socket_cliente, (void *)RESULT_ERROR, sizeof(int), NULL);
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
