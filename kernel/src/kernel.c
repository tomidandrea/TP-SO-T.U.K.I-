#include <kernel.h>
#include <utils/sockets.h>

t_log* logger;
t_config* config;
uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;



int main(int argc, char* argv[]) {
    logger = iniciar_logger("kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    config = iniciar_config(argv[1]);
    //char* ip = config_get_string_value(config,"IP_KERNEL");;
    char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

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
				case PROGRAMA:
					lista = recibir_paquete(socket_cliente); //hace recv de la lista
					t_list* instrucciones = listaAInstrucciones(lista);
					log_info(logger, "Me llego un paquete\n");
					//mandarCpu();
					//list_iterate(lista, (void*) iterator);
					int cant = list_size(instrucciones);
							for(int i = 0;i<cant;i++) {
								t_instruccion* inst = list_get(instrucciones,i);
								log_info(logger, "elemento: %s \n", inst-> instruccion);
								 int cant_parametros = cantParametros(inst->instruccion);

									    	for(int i=0; i<cant_parametros; i++) {
									    		log_info(logger, "Parametro %d: %s", i, inst->parametros[i]);
									    	}
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

/*void mandarCpu(){
	char* ip = config_get_string_value(config,"IP_CPU");
	char* puerto = config_get_string_value(config,"PUERTO_CPU");
	t_socket conexion = crear_conexion(ip, puerto, logger);
}*/

