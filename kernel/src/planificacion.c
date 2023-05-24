#include <planificacion.h>

extern t_config* config;
extern t_log* logger;

extern t_list* procesosExecute;
extern t_list* procesosReady;
extern t_list* procesosNew;

sem_t sem_new_a_ready, sem_ready, sem_grado_multiprogramacion;
pthread_mutex_t mutex_procesos_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_procesos_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_procesos_execute = PTHREAD_MUTEX_INITIALIZER;


void planificar(){
	char* algoritmo = malloc(16);
	t_pcb* proceso;
	algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	while (1)
	{
		sem_wait(&sem_ready);
		 // aca ya los tengo en ready
		log_info(logger, "Empezando a planificar\n");
		//sem_wait(&sem_ready); // Agrego otro semaforo para que no empiece a planificar por ahora
		if(strcmp(algoritmo,"FIFO") == 0){
			proceso = planificarFIFO();
			log_info(logger, "Termine de planificar\n");
		}

		//printf("Instruccion: %s\n", ((t_instruccion*)list_get(proceso -> instrucciones,0))->instruccion);
		mandar_pcb_a_CPU(proceso);
		log_info(logger, "Proceso enviado a cpu\n");

		// RCV de cpu
		actualizar_pcb(proceso);

		// TODO: Quitar proceso de procesosExecute (proceso en running)

		switch(proceso->motivo){
			case EXT:
				log_info(logger, "Salimos como unos campeones\n");
				// Pasar a estado EXIT
				// liberar_recursos();
				// avisar_fin_a_memoria();
				// avisar_fin_a_consola();
				break;
			case YIELD:
				log_info(logger, "Hubo un YIELD\n");

				// Lo agrego al final de la lista de ready

				pthread_mutex_lock(&mutex_procesos_ready);
				list_add(procesosReady, proceso);
				pthread_mutex_unlock(&mutex_procesos_ready);

				// Si algoritmo == HRRN -> calcular_estimado();
				break;
			case WAIT:
				log_info(logger, "Llego un WAIT pibe\n");
				/*if(verificarRecursos(proceso->recurso)){

				}else{
					log_error(logger, "No existe el recurso: %s", proceso->recurso);
				}*/
				break;
			default:
				log_info(logger, "No se implemento xd\n");
				break;
		}
	}
	free(algoritmo);
}

void agregarReady(){
	
	while (1)
	{
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
	t_pcb* proceso;
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

t_pcb* planificarFIFO(){
	//t_pcb* proceso = malloc(sizeof(list_get(procesosReady, 0)));
	t_pcb* proceso = list_remove(procesosReady, 0);
	//t_instruccion* inst = list_get(proceso -> instrucciones,0);
	//printf("Instruccion del fifo: %s\n", inst->instruccion);
	pthread_mutex_lock(&mutex_procesos_execute);
	list_add(procesosExecute, proceso);
	pthread_mutex_unlock(&mutex_procesos_execute);
	return proceso;
}

