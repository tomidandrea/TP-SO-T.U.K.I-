#include<sincro.h>

extern t_config* config;
extern t_log* logger;

extern t_list* procesosExecute;
extern t_list* procesosReady;
extern t_list* procesosNew;

extern sem_t sem_new_a_ready, sem_ready, sem_grado_multiprogramacion, sem_recibir_cpu, sem_recibir_fs, sem_execute;
extern pthread_mutex_t mutex_procesos_new;
extern pthread_mutex_t mutex_procesos_ready;
extern pthread_mutex_t mutex_procesos_execute;
extern pthread_mutex_t mutex_procesos_io;

// creamos los hilos

void crearEscucharConsolas(){
	pthread_t hilo_consolas;
	pthread_create(&hilo_consolas,
					NULL,
					(void*) escucharConsolas,
					NULL);
	pthread_detach(hilo_consolas);
}
void crearAgregarReady(){
	pthread_t hilo_ready;
	pthread_create(&hilo_ready,
					NULL,
					(void*) agregarReady,
					NULL);
	pthread_detach(hilo_ready);
}
void crearPlanificar(){
	pthread_t hilo_planificador;
	pthread_create(&hilo_planificador,
					NULL,
					(void*) planificar,
					NULL);
	pthread_detach(hilo_planificador);

	//while(1);
}

void crearRecibirDeCPU(){
	pthread_t hilo_recibir_cpu;
	pthread_create(&hilo_recibir_cpu,
					NULL,
					(void*) recibirDeCPU,
					NULL);
	pthread_detach(hilo_recibir_cpu);

	//while(1);
}

void crearRecibirDeFS(){
	pthread_t hilo_recibir_fs;
	pthread_create(&hilo_recibir_fs,
					NULL,
					(void*) recibirDeFS,
					NULL);
	pthread_detach(hilo_recibir_fs);

	//while(1);
}
void inicializarSemoforos(){
	sem_init(&sem_new_a_ready, 0, 0); //Binario, iniciado en 0 para que solo agregue a ready si hay procesos en new
	sem_init(&sem_ready, 0, 0); //Binario, solo pase a CPU si hay en ready
	sem_init(&sem_grado_multiprogramacion, 0, config_get_int_value(config,"GRADO_MAX_MULTIPROGRAMACION"));
	sem_init(&sem_execute, 0, 1);
	sem_init(&sem_recibir_cpu, 0, 0);
	sem_init(&sem_recibir_fs, 0, 0);
}


void liberarSemoforos(){
	sem_destroy(&sem_new_a_ready);
	sem_destroy(&sem_ready);
	sem_destroy(&sem_grado_multiprogramacion);
	sem_destroy(&sem_execute);
	sem_destroy(&sem_recibir_cpu);
	sem_destroy(&sem_recibir_fs);
	liberarMutex();
}

void liberarMutex(){ //Semaforos mutex para acceder a las listas de procesos
	pthread_mutex_destroy(&mutex_procesos_ready);
	pthread_mutex_destroy(&mutex_procesos_new);
	pthread_mutex_destroy(&mutex_procesos_execute);
	pthread_mutex_destroy(&mutex_procesos_io);
}

void pasarNewAReady(){
	t_pcb* proceso;
	/*int cant = list_size(procesosNew);
	for(int i = 0;i<cant;i++) {
		proceso = list_get(procesosNew,i);
		log_info(logger, "elemento %d: %d \n", i, proceso->pid);
	}*/
	pthread_mutex_lock(&mutex_procesos_new);
	proceso = list_remove(procesosNew, 0);
	pthread_mutex_unlock(&mutex_procesos_new);

	pasarAReady(proceso);
	log_info(logger, "PID: %d - Estado Anterior: NEW - Estado Actual: READY", proceso->pid);
}

void pasarAReady(t_pcb* proceso){
	pthread_mutex_lock(&mutex_procesos_ready);
	list_add(procesosReady, proceso);
	pthread_mutex_unlock(&mutex_procesos_ready);
	//proceso->tiempoEnReady = iniciarTiempo();
	temporal_resume(proceso->tiempoEnReady);
	//log_info(logger, "Paso Proceso %d a ready", proceso->pid);
	sem_post(&sem_ready);
}

void pasarAExecute(t_pcb* proceso){
	pthread_mutex_lock(&mutex_procesos_execute);
	list_add(procesosExecute, proceso);
	pthread_mutex_unlock(&mutex_procesos_execute);

}

void agregarAlInicioDeReady(t_pcb* proceso) {
	pthread_mutex_lock(&mutex_procesos_ready);
	list_add_in_index(procesosReady,0,proceso);
	pthread_mutex_unlock(&mutex_procesos_ready);
	sem_post(&sem_ready);
}

t_pcb* removerPrimeroDeReady(){
	t_pcb* proceso;
	pthread_mutex_lock(&mutex_procesos_ready);
	proceso = list_remove(procesosReady, 0);
	pthread_mutex_unlock(&mutex_procesos_ready);
	return proceso;
}

t_pcb* removerDeExecute() {
    pthread_mutex_lock(&mutex_procesos_execute);
	t_pcb* proceso = list_remove(procesosExecute, 0);
	pthread_mutex_unlock(&mutex_procesos_execute);
	log_debug(logger, "Proceso que sacamos de execute:%d\n", proceso->pid);
	sem_post(&sem_execute);
	return proceso;
}

bool compararProcesoDeExecute(int pid) {
    pthread_mutex_lock(&mutex_procesos_execute);
	t_pcb* proceso = list_get(procesosExecute, 0);
	pthread_mutex_unlock(&mutex_procesos_execute);
	return proceso->pid==pid;
}


