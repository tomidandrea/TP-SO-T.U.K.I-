#include <kernel.h>
#include <utils/sockets.h>

int main(int argc, char* argv[]) {
    puts("Hello world!!");
    t_log* logger = iniciar_logger();
    t_config* config = iniciar_config(argv[1]);;
    //char* ip = config_get_string_value(config,"IP_KERNEL");;
    char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

	t_socket server_fd = iniciar_servidor(puerto, logger);
	t_socket cliente_fd = esperar_cliente(server_fd, logger); //Hace el accept

	t_list* lista;
		//while (1) {
			int cod_op = recibir_operacion(cliente_fd); //hace recv del cod_op

			switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(cliente_fd, logger);
				break;
			case PAQUETE:
				lista = recibir_paquete(cliente_fd); //hace recv de la lista
				log_info(logger, "Me llegaron los siguientes valores:\n");
				//list_iterate(lista, (void*) iterator);
				//logearInstrucciones(lista, logger);
				int cant = list_size(lista);
						for(int i = 0;i<cant;i++) {
							log_info(logger, "elemento: %s \n", list_get(lista,i));
						}
				break;
			case -1:
				send(cliente_fd, (void *)RESULT_ERROR, sizeof(int), NULL);
				log_error(logger, "el cliente se desconecto. Terminando servidor");
				return EXIT_FAILURE;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
			//}
		}

		send(cliente_fd, (void *)RESULT_OK, sizeof(int), NULL);
		return EXIT_SUCCESS;
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
