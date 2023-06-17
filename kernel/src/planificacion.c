#include <planificacion.h>

extern t_config* config;
extern t_log* logger;

extern t_list* procesosExecute;
extern t_list* procesosReady;
extern t_list* procesosNew;

uint32_t INICIO = 1;

sem_t sem_new_a_ready, sem_ready, sem_grado_multiprogramacion, sem_recibir, sem_execute;
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

		sem_wait(&sem_execute);
		sem_wait(&sem_ready);

		log_info(logger, "Empezando a planificar");
		pthread_mutex_lock(&mutex_procesos_ready);
		char* mensaje = lista_procesos_string(procesosReady);
		pthread_mutex_unlock(&mutex_procesos_ready);
		log_info(logger, "Cola Ready %s: [%s]", algoritmo, mensaje);
		free(mensaje);
		//mostrarListaProcesos(procesosReady);
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
		sem_post(&sem_recibir);

	}
	free(algoritmo);
}

void recibirDeCPU() {

	while(1) {
	sem_wait(&sem_recibir);
	t_pcb* proceso = list_get(procesosExecute, 0);
	t_contexto* contexto = actualizar_pcb(proceso);
	char* recurso;
	int id;

		switch(contexto->motivo){
			case EXT:
				log_info(logger, "=== Salimos como unos campeones!!!!, PID:%d finalizó ===\n", proceso->pid);
				proceso = removerDeExecute();
				log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", proceso->pid);
				// liberar_recursos();
				// avisar_fin_a_memoria();
				avisar_fin_a_consola(proceso->socket_consola);
				liberar_pcb(proceso);
				log_debug(logger, "Lista procesosReady:%d", list_size(procesosReady));
				log_debug(logger, "POST grado multi");
				sem_post(&sem_grado_multiprogramacion);
				break;
			case YIELD:
				log_info(logger, "Hubo un YIELD del proceso %d\n", proceso->pid);

				proceso = removerDeExecute();
				pasarAReady(proceso);
				log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: READY", proceso->pid);

				break;
			case IO:
				log_info(logger, "Hubo un IO de PID:%d\n", contexto->pid);
				int tiempo = atoi(contexto->parametros[0]);
				proceso = removerDeExecute();
				io_contexto* ioContexto = inicializarIoContexto(proceso, tiempo);
				ejecutarIO(ioContexto);
				break;
			case WAIT:
				//proceso = removerDeExecute();
				log_info(logger, "Llego un WAIT pibe\n");
				recurso = contexto->parametros[0];
				if(verificarRecursos(recurso)){
					wait(proceso, recurso);
				} else{
					avisar_fin_a_consola(proceso->socket_consola);
					sem_post(&sem_grado_multiprogramacion);
					log_error(logger, "No existe el recurso: %s", recurso);
					log_error(logger, "Finalizo proceso PID: %d", proceso->pid);
					log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", proceso->pid);
				}
				break;
			case SIGNAL:
				//INICIO = 1; //para que no haga el calculo HRRN y agarre el primero de ready
				//proceso = removerDeExecute();
				log_info(logger, "Llego un SIGNAL pibe\n");
				recurso = contexto->parametros[0];
				if(verificarRecursos(recurso)){
					ejecutarSignal(proceso, recurso);
				} else{
					avisar_fin_a_consola(proceso->socket_consola);
					sem_post(&sem_grado_multiprogramacion);
					log_error(logger, "No existe el recurso: %s", recurso);
					log_error(logger, "Finalizo proceso PID: %d", proceso->pid);
					log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", proceso->pid);
					//FINALIZAR PROCESO
				}
				break;
			case CREATE_SEGMENT:
				log_info(logger, "Llego un CREATE_SEGMENT pibe\n");
				id = atoi(contexto->parametros[0]);
				int tamanio = atoi(contexto->parametros[1]);
				solicitarCrearSegmento(id,tamanio, proceso); //mandamos a memoria
				log_info(logger, "PID: %d - Crear Segmento - Id: %d - Tamaño: %d", proceso->pid, id, tamanio);
				recibirCrearSegmento(id, tamanio, proceso);
				break;
			case DELETE_SEGMENT:
				log_info(logger, "Llego un DELETE_SEGMENT pibe\n");
				id = atoi(contexto->parametros[0]);
				//TODO verificar que exista el segmento y no sea el 0
				eliminarSegmento(id, proceso);
				log_info(logger, "PID: %d - Eliminar Segmento - Id: %d", proceso->pid, id);

				recibirTablaActualizada(proceso);
				avisar_fin_a_consola(proceso->socket_consola);

				break;
			default:
				log_debug(logger, "No se implemento la instruccion");
				break;
		}
		liberar_contexto(contexto);
	}
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
	log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);
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

	log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);

	return proceso;

}


