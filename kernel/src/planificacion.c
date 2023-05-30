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

	while (1){
		sem_wait(&sem_ready);
		log_info(logger, "Empezando a planificar");
		//sem_wait(&sem_ready); // Agrego otro semaforo para que no empiece a planificar por ahora

		if(strcmp(algoritmo,"FIFO") == 0){
			proceso = planificarFIFO();
			log_info(logger, "Termine de planificar FIFO");
		}else if(strcmp(algoritmo,"HRRN") == 0){
			proceso = planificarHRRN(alfa);
			log_info(logger, "Termine de planificar HRRN");
		}else{
			log_error(logger, "Nombre del algoritmo invalido");
		}

		mandar_pcb_a_CPU(proceso);
		log_info(logger, "Proceso %d enviado a cpu\n", proceso->pid);
		t_contexto* contexto = actualizar_pcb(proceso);

		// TODO: Quitar proceso de procesosExecute (proceso en running)

		switch(contexto->motivo){
			case EXT:
				log_info(logger, "=== Salimos como unos campeones!!!!, PID:%d finalizó ===\n", proceso->pid);
				proceso = removerDeExecute();
				// Pasar a estado EXIT
				// liberar_recursos();
				// avisar_fin_a_memoria();
				avisar_fin_a_consola(proceso->socket_consola);
				log_debug(logger, "Lista procesosReady:%d", list_size(procesosReady));
				log_debug(logger, "POST grado multi");
				sem_post(&sem_grado_multiprogramacion);
				break;
			case YIELD:
				log_info(logger, "Hubo un YIELD del proceso %d\n", proceso->pid);

				//proceso->tiempoEnReady = iniciarTiempo();
				//temporal_resume(proceso->tiempoEnReady);
				// Lo agrego al final de la lista de ready
				proceso = removerDeExecute();
				pasarAReady(proceso);

				// Si algoritmo == HRRN -> calcular_estimado();
				break;
			case IO:
				log_info(logger, "Hubo un IO de PID:%d\n", contexto->pid);
				int tiempo = atoi(contexto->parametros[0]);
				proceso = removerDeExecute();
				io_contexto* ioContexto = inicializarIoContexto(proceso, tiempo);
				ejecutarIO(ioContexto);
				break;
			case WAIT:
				proceso = removerDeExecute();
				log_info(logger, "Llego un WAIT pibe\n");
				char* recursoW = contexto->parametros[0];
				if(verificarRecursos(recursoW)){
					INICIO = wait(proceso, recursoW);
				} else{
					log_error(logger, "No existe el recurso: %s", recursoW);
					// TODO:FINALIZAR PROCESO
				}
				break;
			case SIGNAL:
				INICIO = 1; //para que no haga el calculo HRRN y agarre el primero de ready
				proceso = removerDeExecute();
				log_info(logger, "Llego un SIGNAL pibe\n");
				char* recursoS = contexto->parametros[0];
				if(verificarRecursos(recursoS)){
					ejecutarSignal(proceso, recursoS);
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
		pasarNewAReady();
	}

}



t_pcb* planificarFIFO(){
	t_pcb* proceso = removerPrimeroDeReady();
	pasarAExecute(proceso);
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

	t_pcb* proceso = inicializar_pcb();

	if(INICIO){
		proceso = removerPrimeroDeReady();
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
			log_debug(logger, "Proceso %d tiempoEnReady : %ld", proceso->pid ,tiempoEnReady);
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

	pasarAExecute(proceso);

	return proceso;

}


