#include <manejoRecursos.h>

int cantidad_recursos;

extern t_config* config;
extern t_log* logger;

extern char** instanciasRecursos;
extern int* instancias;
extern t_list* colasDeBloqueados;
extern char** recursos;
extern sem_t sem_recibir;


void inicializarRecursos(){
	char* recursos_config =  config_get_string_value(config, "RECURSOS");
	cantidad_recursos = contar(recursos_config, ',') + 1; //cuento las comas y le sumo uno para saber cantidad de recursos
	//TODO: Falta un if para ver si no hay recursos!!

	//log_info(logger, "Existen %d recursos: %s\n", cantidad_recursos, recursos_config);
	recursos = inicializar_parametros(cantidad_recursos);
	recursos = config_get_array_value(config, "RECURSOS");
	instanciasRecursos = inicializar_parametros(cantidad_recursos);
	instanciasRecursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");
	instancias = malloc(sizeof(int) * cantidad_recursos);
	pasarAInstanciasEnteras();
	for (int i = 0; i < cantidad_recursos; ++i) {
		log_info(logger, "Recurso %d: %s tiene %d instancia/s", i, recursos[i], instancias[i]);
	}
}

void pasarAInstanciasEnteras() {
	for(int i=0; i < cantidad_recursos; i++) {
        instancias[i] = atoi(instanciasRecursos[i]);
    }
}

int verificarRecursos(char* recurso){
	int existeRecurso = RECURSO_INEXISTENTE;
	//Verificamos que el recurso exista
	for(int i = 0; i<cantidad_recursos;i++){
		if(strcmp(recurso, recursos[i]) == 0){
			existeRecurso = RECURSO_EXISTENTE;
			break;
		}
	}
	return existeRecurso;
}

int cantInstancias(char* recurso) {
	int i =  indice(recurso);
	return instancias[i];
}

int indice(char* recurso) {
    for(int i=0; i < cantidad_recursos; i++) {
        if(strcmp(recursos[i], recurso) == 0) return i;
    }
    return -1;
}

void wait(t_pcb* proceso, char* recurso) {

    disminuirInstancias(recurso);
    int instancias = cantInstancias(recurso);
    log_info(logger, "PID: %d - Wait: %s - Instancias: %d", proceso->pid, recurso, instancias);
    if(instancias < 0) {
    	removerDeExecute();
        bloquear(proceso, recurso);
    } else {
		//agregarAlInicioDeReady(proceso);
		log_info(logger, "Proceso %d vuelve a cpu por disponibilidad del recurso %s\n", proceso->pid, recurso);
		mandar_pcb_a_CPU(proceso);
		sem_post(&sem_recibir);
    }
}

void ejecutarSignal(t_pcb* proceso, char* recurso) {
    aumentarInstancias(recurso);
    int instancias = cantInstancias(recurso);
    log_info(logger, "PID: %d - Signal: %s - Instancias: %d", proceso->pid, recurso, instancias);
    if(instancias <= 0) {
         desbloquearPrimerProceso(recurso);
    }
	//agregarAlInicioDeReady(proceso);
	log_info(logger, "Se realizo signal del recurso %s. El proceso %d puede seguir ejecutandose en cpu\n", recurso, proceso->pid);
	mandar_pcb_a_CPU(proceso);
	sem_post(&sem_recibir);

}

io_contexto* inicializarIoContexto(t_pcb* proceso, int tiempo){
	io_contexto* contexto = malloc(sizeof(io_contexto));
	contexto->proceso = proceso;
	contexto->tiempo_sleep = tiempo;
	return contexto;
}

void ejecutarIO(io_contexto* contexto) {
	pthread_t hilo_bloqueoPorIO;
	log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: BLOCKED", contexto->proceso->pid);
	log_info(logger, "PID: %d - Bloqueado por: IO", contexto->proceso->pid);
	log_info(logger, "PID: %d - Ejecuta IO: %d\n",contexto->proceso->pid, contexto->tiempo_sleep);
	pthread_create(&hilo_bloqueoPorIO,NULL,(void*)bloquearYPasarAReady,contexto);
	pthread_detach(hilo_bloqueoPorIO);
}

void bloquearYPasarAReady(io_contexto* contexto) {
	sleep(contexto->tiempo_sleep);
	//log_info(logger,"Finaliza bloqueo de Proceso %d por IO y pasa a estado ready", contexto->proceso->pid);
	log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", contexto->proceso->pid);
	pasarAReady(contexto->proceso);
	//sem_post(&sem_ready);
}

void crearColasDeBloqueados() {
    for(int i=0; i < cantidad_recursos; i++) {
        t_queue* cola = queue_create();
        list_add(colasDeBloqueados, cola);
    }
}

void aumentarInstancias(char* recurso) {
    int i =  indice(recurso);
    instancias[i] += 1;
    log_info(logger, "La nueva cantidad de instancias del recurso %s es: %d", recurso,instancias[i]);
}

void disminuirInstancias(char* recurso) {
    int i =  indice(recurso);
    instancias[i] -= 1;
    log_info(logger, "La nueva cantidad de instancias del recurso %s es: %d", recurso,instancias[i]);
}

void desbloquearPrimerProceso(char* recurso) {
    int i = indice(recurso);
    t_queue* cola = list_get(colasDeBloqueados, i);
    t_pcb* proceso = queue_peek(cola);
    pasarAReady(proceso);
    log_info(logger, "Se desbloqueo el proceso %d de la cola del recurso %s",proceso->pid, recurso);
    log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", proceso->pid);
    queue_pop(cola);

}

void bloquear(t_pcb* proceso, char* recurso) {
    int i = indice(recurso);
    t_queue* cola = list_get(colasDeBloqueados, i);
    queue_push(cola, proceso);

    log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: BLOCKED", proceso->pid);
    log_info(logger, "PID: %d - Bloqueado por: %s", proceso->pid, recurso);

}


