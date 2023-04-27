#include <kernel.h>


t_log* logger;
t_config* config;
t_list* procesosExecute;
t_list* procesosReady;
t_list* procesosNew;

//todo: mallocs y free para todos los punteros
//todo hice malloc de 16 pero no se si estan bien


int main(int argc, char* argv[]) {
    logger = iniciar_logger("kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    config = iniciar_config(argv[1]);
    inicializarSemoforos();

    // todo: settear esto en una funcion aparte setConfig(config)
    /*char* ip = malloc(16);
    ip = config_get_string_value(config,"IP_KERNEL");
    char* puerto = malloc(16);
    puerto = config_get_string_value(config,"PUERTO_ESCUCHA");
*/
    //free(ip);

    procesosNew = list_create();
    procesosReady = list_create();
    procesosExecute = list_create();

	t_list* lista;

	// Hilo que atiende consolas
		
			
			
		pthread_t hilo_consolas;
		/*hilo_consolas_args* arg = malloc(sizeof(hilo_consolas_args));
				arg->cliente = socket_cliente;
				arg->lista = lista;*/
		pthread_create(&hilo_consolas,
						NULL,
						(void*) escucharConsolas,
						//(void *)arg);
						NULL);
		pthread_detach(hilo_consolas);
		

		pthread_t hilo_ready;
		pthread_create(&hilo_ready,
						NULL,
						(void*) agregarReady,
						NULL);
		pthread_detach(hilo_ready);

		pthread_t hilo_planificador;
		pthread_create(&hilo_planificador,
						NULL,
						(void*) planificar,
						NULL);
		pthread_detach(hilo_planificador);
		//escucharConsolas();
		//while(escucharConsolas());

		while(1);

		return EXIT_SUCCESS;
}
/*
void atender_cliente(hilo_consolas_args* argumentos){
	int socket_cliente = argumentos->cliente;
	t_list* lista = argumentos->lista;
	int cod_op = recibir_operacion(socket_cliente); //hace recv del cod_op

				switch (cod_op) {
				case PAQUETE:
					lista = recibir_paquete(socket_cliente); //hace recv de la lista
					log_info(logger, "Me llego un paquete\n");

					
					t_pcb* pcb = crearPCB(lista);

					list_add(procesosNew, pcb);
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
}*/

void mandarCpu(){
	char* ip = config_get_string_value(config,"IP_CPU");
	char* puerto = config_get_string_value(config,"PUERTO_CPU");
	t_socket conexion = crear_conexion(ip, puerto, logger);
}

