#include <memoria.h>

int main(int argc, char* argv[]) {
	t_log* logger;
	t_config* config;
	logger = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);
	char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

	t_socket server_fd = iniciar_servidor(puerto, logger);
	while(1){
		t_socket socket_cliente = esperar_cliente(server_fd, logger);
	}
}
