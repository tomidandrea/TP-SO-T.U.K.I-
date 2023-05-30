#include <planificacion.h>

extern t_config* config;
extern t_log* logger;

extern t_list* procesosExecute;
extern t_list* procesosReady;
extern t_list* procesosNew;

uint32_t INICIO = 1;

sem_t sem_new_a_ready, sem_ready, sem_grado_multiprogramacion;
pthread_mutex_t mutex_procesos_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_procesos_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_procesos_execute = PTHREAD_MUTEX_INITIALIZER;


void planificar(){
	char* algoritmo = malloc(16);
	t_pcb* proceso;
	algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	double alfa;
	alfa = config_get_double_value(config,"HRRN_ALFA");

	while (1)
	{
		sem_wait(&sem_ready);
		log_info(logger, "Empezando a planificar\n");
		//sem_wait(&sem_ready); // Agrego otro semaforo para que no empiece a planificar por ahora

		if(strcmp(algoritmo,"FIFO") == 0){
			proceso = planificarFIFO();
			int64_t tiempo = temporal_gettime(proceso->tiempoEnReady);
			log_info(logger, "Tiempo en ready: %ld\n", tiempo);
			log_info(logger, "Termine de planificar FIFO\n");
		}

		if(strcmp(algoritmo,"HRRN") == 0){
					proceso = planificarHRRN(alfa);

					log_info(logger, "Termine de planificar HRRN\n");
				}

		mandar_pcb_a_CPU(proceso);
		log_info(logger, "Proceso enviado a cpu\n");
		t_contexto* contexto = actualizar_pcb(proceso);

		// TODO: Quitar proceso de procesosExecute (proceso en running)

		switch(contexto->motivo){
			case EXT:
				log_info(logger, "Salimos como unos campeones\n");
				sacarDeCPU();
				// Pasar a estado EXIT
				// liberar_recursos();
				// avisar_fin_a_memoria();
				// avisar_fin_a_consola();
				printf("Lista procesosReady:%d\n", list_size(procesosReady));
				if(!list_is_empty(procesosReady)){
					printf("Entre al if\n");
					sem_post(&sem_ready);
				}
				printf("POST grado multi\n");
				sem_post(&sem_grado_multiprogramacion);
				break;
			case YIELD:
<<<<<<< HEAD
				log_info(logger, "Hubo un YIELD del proceso %d\n", proceso->pid);

				//proceso->tiempoEnReady = iniciarTiempo();
				temporal_resume(proceso->tiempoEnReady);
=======
				log_info(logger, "Hubo un YIELD\n");

				// Lo agrego al final de la lista de ready
				sacarDeCPU();
>>>>>>> cpu
				pthread_mutex_lock(&mutex_procesos_ready);
				list_add(procesosReady, proceso);
				pthread_mutex_unlock(&mutex_procesos_ready);

				sem_post(&sem_ready);

				// Si algoritmo == HRRN -> calcular_estimado();
				break;
			case IO:
				log_info(logger, "Hubo un IO\n");
				int tiempo = atoi(contexto->parametros[0]);
				ejecutarIO(tiempo);
				break;
			case WAIT:
				log_info(logger, "Llego un WAIT pibe\n");
				char* recursoW = contexto->parametros[0];
				if(verificarRecursos(recursoW)){
					wait(recursoW);
				} else{
					log_error(logger, "No existe el recurso: %s", recursoW);
					// TODO:FINALIZAR PROCESO
				}
				break;
			case SIGNAL:
				log_info(logger, "Llego un SIGNAL pibe\n");
				char* recursoS = contexto->parametros[0];
				if(verificarRecursos(recursoS)){
					ejecutarSignal(recursoS);
				} else{
					log_error(logger, "No existe el recurso: %s", recursoS);
					//FINALIZAR PROCESO
				}
				break;
			default:
				log_info(logger, "No se implemento xd\n");
				break;
		}
		liberar_contexto(contexto);
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
	//proceso->tiempoEnReady = iniciarTiempo();
	temporal_resume(proceso->tiempoEnReady);
}

t_pcb* planificarFIFO(){
	t_pcb* proceso = list_remove(procesosReady, 0);

	pthread_mutex_lock(&mutex_procesos_execute);
	list_add(procesosExecute, proceso);
	pthread_mutex_unlock(&mutex_procesos_execute);

	return proceso;
}

int procesoConMayorRatio(int cant){
	double ratio = 0;
	t_pcb* proceso;
	int max;
	for(int i=0;i<cant;i++){
		proceso = list_get(procesosReady,i);
		if(ratio < proceso->ratio){
			ratio = proceso->ratio;
			max = i;
		}
	}
	return max;
}

t_pcb* planificarHRRN(double alfa){

	t_pcb* proceso;

	if(INICIO){
		proceso = list_remove(procesosReady, 0);
		proceso->tiempoEnReady= pararTiempo(proceso->tiempoEnReady);
		//pararTiempoReady(proceso);
		INICIO = 0;
	}
	else {
		int64_t tiempoEnReady;
		double est;
		double estActual;
		int cant = list_size(procesosReady);

		for(int i = 0;i<cant;i++) {
			proceso = list_get(procesosReady,i);
			tiempoEnReady = temporal_gettime(proceso->tiempoEnReady);
			printf("Proceso %d tiempoEnReady : %ld \n", proceso->pid ,tiempoEnReady);
			est = proceso->estimadoAnterior;

			// calculo la rafaga actual
			if(proceso->pc == 0){
				estActual = proceso->estimadoAnterior;
			}
			else{
				int64_t realEjec;
				realEjec = temporal_gettime(proceso->tiempoCPU);
				proceso->tiempoCPU = pararTiempo(proceso->tiempoCPU);
				//pararTiempoCPU(proceso);

				estActual = alfa*realEjec + (1-alfa)*est;

				proceso->estimadoAnterior = estActual;
				//temporal_resume(proceso->tiempoCPU);
				//printf("realEjec %ld \n", realEjec);
				//printf("proceso->tiempoCPU %ld \n", temporal_gettime(proceso->tiempoCPU));

			}

			// calculo el ratio
			proceso->ratio = (tiempoEnReady + estActual)/estActual;
			log_info(logger,"RATIO: %f - proceso: %d", proceso->ratio, proceso->pid);

		}
		// aca se puede usar la funcion de las commons list_get_maximum
		proceso = list_remove(procesosReady, procesoConMayorRatio(cant));
		proceso->tiempoEnReady = pararTiempo(proceso->tiempoEnReady);
		//pararTiempoReady(proceso);
	}
	pthread_mutex_lock(&mutex_procesos_execute);
	list_add(procesosExecute, proceso);
	pthread_mutex_unlock(&mutex_procesos_execute);

	return proceso;

}


