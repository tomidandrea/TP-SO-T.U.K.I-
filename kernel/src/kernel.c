#include <kernel.h>


t_log* logger;
t_config* config;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;
t_list* procesos;

int main(int argc, char* argv[]) {
    logger = iniciar_logger("kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    config = iniciar_config(argv[1]);
    //char* ip = config_get_string_value(config,"IP_KERNEL");;
    char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");
    char* algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    char* gradoMultipAux = config_get_string_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    int gradoMultip = atoi(gradoMultipAux);

    procesos = list_create();
	t_socket server_fd = iniciar_servidor(puerto, logger);
	t_list* lista;
		while (1) {
			t_socket socket_cliente = esperar_cliente(server_fd, logger); //Hace el accept
			if(socket_cliente != -1){
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

			if(strcmp(algoritmo,"FIFO") == 0){
				planificarFIFO(procesos, gradoMultip);
			}

	}
		return EXIT_SUCCESS;
}

void atender_cliente(thread_args* argumentos){
	int socket_cliente = argumentos->cliente;
	t_list* lista = argumentos->lista;
	int cod_op = recibir_operacion(socket_cliente); //hace recv del cod_op

				switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(socket_cliente, logger);
					break;
				case PAQUETE:
					lista = recibir_paquete(socket_cliente); //hace recv de la lista
					log_info(logger, "Me llego un paquete\n");

					t_pcb* pcb = crearPCB(lista);

					list_add(procesos, pcb);
					printf("\nAgregue el proceso:%d\n",pcb->pid);

					//list_iterate(lista, (void*) iterator);
					int cant = list_size(lista);
							for(int i = 0;i<cant;i++) {
								log_info(logger, "elemento: %s \n", list_get(lista,i));
							}
					break;
				case -1:
					//send(socket_cliente, (void *)RESULT_ERROR, sizeof(int), NULL);
					log_error(logger, "el cliente se desconecto. Terminando servidor");
					//return EXIT_FAILURE;
					break;

				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
				}

		send(socket_cliente, &RESULT_OK, sizeof(int), NULL);
		close(socket_cliente);
}


