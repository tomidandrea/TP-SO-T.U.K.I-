#include <memoria.h>

t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {
	logger = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);

	t_socket server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");
	while(1){
		t_socket socket_cliente = esperar_cliente(server_fd, logger);
	}
}
