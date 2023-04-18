#include <kernel.h>
#include <utils/sockets.h>

int main(int argc, char* argv[]) {
    t_log* logger = iniciar_logger();
    t_config* config = iniciar_config(argv[1]);;
    //char* ip = config_get_string_value(config,"IP_KERNEL");;
    char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

	t_socket server_fd = iniciar_servidor(puerto, logger);
	t_list* lista;
		while (1) {
			t_socket socket_cliente = esperar_cliente(server_fd, logger); //Hace el accept
			if(socket_cliente != 1){
				pthread_t thread;
				thread_args* arg = malloc(sizeof(thread_args));
						arg->cliente = socket_cliente;
						arg->logger = logger;
						arg->lista = lista;
				pthread_create(&thread,
							  NULL,
							  (void*) atender_cliente,
							  (void *)arg);
				pthread_detach(thread);
			}
	}
		return EXIT_SUCCESS;
}

void atender_cliente(thread_args* argumentos){
//(int socket_cliente, t_log* logger, t_list* lista){
	int socket_cliente = argumentos->cliente;
	t_log* logger = argumentos->logger;
	t_list* lista = argumentos->lista;
		int cod_op = recibir_operacion(socket_cliente); //hace recv del cod_op

				switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(socket_cliente, logger);
					break;
				case PAQUETE:
					lista = recibir_paquete(socket_cliente); //hace recv de la lista
					log_info(logger, "Me llego un paquete\n");
					//list_iterate(lista, (void*) iterator);
					//int cant = list_size(lista);
							/*for(int i = 0;i<cant;i++) {
								log_info(logger, "elemento: %s \n", list_get(lista,i));
							}*/
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

		send(socket_cliente, (void *)RESULT_OK, sizeof(int), NULL);
		close(socket_cliente);
}


t_log* iniciar_logger(void)
{
	char* file = "kernel.log";
	char *process_name = "KERNEL";
	bool is_active_console = true;
	t_log_level level = LOG_LEVEL_DEBUG;
	t_log* nuevo_logger = log_create(file, process_name,is_active_console, level);

	return nuevo_logger;
}

t_config* iniciar_config (char* path){
	t_config* nuevo_config = config_create(path);
	if(nuevo_config== NULL){
		printf("No se pudo crear el config\n");
	}

	return nuevo_config;
}
