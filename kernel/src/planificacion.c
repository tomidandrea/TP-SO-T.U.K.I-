#include <planificacion.h>

extern t_config* config;
extern t_log* logger;

uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

extern t_list* procesosExecute;
extern t_list* procesosReady;
extern t_list* procesosNew;

sem_t sem_new_a_ready, sem_ready,
	//mutex_procesos_new, mutex_procesos_ready, mutex_procesos_execute,
	sem_grado_multiprogramacion;
pthread_mutex_t mutex_procesos_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_procesos_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_procesos_execute = PTHREAD_MUTEX_INITIALIZER;
//haceme un init de cada sem_t en una funcion aparten llamada inicializarSemaforos()


void inicializarSemoforos(){
	sem_init(&sem_new_a_ready, 0, 0); //Binario, iniciado en 0 para que solo agregue a ready si hay procesos en new
	sem_init(&sem_ready, 0, 0); //Binario, solo pase a CPU si hay en ready
	//sem_init(&mutex_procesos_new, 0, 1);
	//sem_init(&mutex_procesos_ready, 0, 1);
	sem_init(&sem_grado_multiprogramacion, 0, config_get_int_value(config,"GRADO_MAX_MULTIPROGRAMACION"));
	//sem_init(&mutex_procesos_execute, 0, 1);
	
}

int escucharConsolas(){
	t_list* lista = malloc(sizeof(t_list));
	char* puerto = malloc(16);
	puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

	t_socket server_fd = iniciar_servidor(puerto, logger);
	printf("\nSocket conexion:%d \n",server_fd);

	free(puerto);

//	int socket_cliente = argumentos->cliente;
	t_socket socket_cliente = malloc(sizeof(int));
	while(1){
	socket_cliente = esperar_cliente(server_fd, logger); //Hace el accept

    printf("\nsocket conexion2:%d \n",server_fd);
		if(socket_cliente != -1){
				int cod_op = recibir_operacion(socket_cliente); //hace recv del cod_op
				switch (cod_op) {
						case PAQUETE:
							lista = recibir_paquete(socket_cliente); //hace recv de la lista
							log_info(logger, "Me llego un paquete\n");
							//send(socket_cliente, &RESULT_OK, sizeof(int), NULL);

							t_pcb* pcb = crearPCB(lista);
							pthread_mutex_lock(&mutex_procesos_new);
							list_add(procesosNew, pcb);
							pthread_mutex_unlock(&mutex_procesos_new);

							//printf("\nAgregue el proceso:%d\n",pcb->pid);
							log_info(logger, "Se crea el proceso:%d en NEW", pcb->pid);
							sem_post(&sem_new_a_ready);

							//list_iterate(lista, (void*) iterator);
							/*int cant = list_size(lista);
									for(int i = 0;i<cant;i++) {
										log_info(logger, "elemento: %s \n", list_get(lista,i));
									}*/
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
		}
	}
	return 0;
	//TODO: hacer que salga del while
	//send(socket_cliente, &RESULT_OK, sizeof(int), NULL);
	//close(socket_cliente);
	free(lista);
}

void planificar(){
	char* algoritmo = malloc(16);
	algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	while (1)
	{
		sem_wait(&sem_ready);
		 // aca ya los tengo en ready
		log_info(logger, "Empezando a planificar\n");
		//sem_wait(&sem_ready); // Agrego otro semaforo para que no empiece a planificar por ahora
		if(strcmp(algoritmo,"FIFO") == 0){
			planificarFIFO();
			log_info(logger, "Termine de planificar\n");
		}

	}
	free(algoritmo);
}

void agregarReady(){
	
	

	while (1)
	{	//TODO: agregar signal en hilo de consola-new
		sem_wait(&sem_new_a_ready);
		sem_wait(&sem_grado_multiprogramacion);
		log_info(logger, "Permite agregar proceso a ready por grado de multiprogramacion\n");
		pasarAReady();
		log_info(logger, "Paso un proceso a ready\n");
		sem_post(&sem_ready);
	}

   
	//aca va HRRN
	//if(strcmp(algoritmo,"HRRN") == 0){
	//	planificarHRRN(procesosNew, procesosReady, procesosExecute, gradoMultip);
	//}

	// esperamos la cpu
	//free(proceso);
}

void pasarAReady(){
	t_pcb* proceso = malloc(sizeof(t_pcb*));
	/*int cant = list_size(procesosNew);
	for(int i = 0;i<cant;i++) {
		proceso = list_get(procesosNew,i);
		log_info(logger, "elemento %d: %d \n", i, proceso->pid);
	}*/
	pthread_mutex_lock(&mutex_procesos_new);
	proceso = list_remove(procesosNew, 0);
	pthread_mutex_unlock(&mutex_procesos_new);
	
	pthread_mutex_lock(&mutex_procesos_ready);
	list_add(procesosReady, proceso);
	pthread_mutex_unlock(&mutex_procesos_ready);
}

void liberarSemoforos(){
	sem_destroy(&sem_new_a_ready);
	//sem_destroy(&mutex_procesos_new);
	//sem_destroy(&mutex_procesos_ready);
	sem_destroy(&sem_grado_multiprogramacion);
	
}



void planificarFIFO(){

	t_pcb* proceso = malloc(sizeof(list_get(procesosReady, 0)));
	proceso = list_remove(procesosReady, 0);
	pthread_mutex_lock(&mutex_procesos_execute);
	list_add(procesosExecute, proceso);
	pthread_mutex_unlock(&mutex_procesos_execute);

	// mandamo la cosa
	//TODO: descomentar para mandar proceso
	//mandar_pcb_a_CPU(proceso);

}

