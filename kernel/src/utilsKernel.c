#include "utilsKernel.h"

#include <comunicacion.h>

int processID=0;
extern t_config* config;
extern t_log* logger;
extern t_socket conexionCPU;

extern t_list* colasDeBloqueados;
extern char** recursos;
extern char** instanciasRecursos;
extern int cantidad_recursos;
extern int* instancias;
extern t_list* procesosExecute;
extern t_list* procesosReady;

extern sem_t sem_new_a_ready, sem_ready, sem_grado_multiprogramacion;
extern pthread_mutex_t mutex_procesos_new;
extern pthread_mutex_t mutex_procesos_ready;
extern pthread_mutex_t mutex_procesos_execute;

t_pcb* crearPCB(t_list* listaInstrucciones){

	t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->pid = ++processID;
    pcb->pc = 0;
    pcb->instrucciones = list_duplicate(listaInstrucciones);
    pcb->registros = inicializarRegistros();
    strcpy(pcb->registros->AX, "0");
    strcpy(pcb->registros->BX, "0");
    strcpy(pcb->registros->CX, "0");
    strcpy(pcb->registros->DX, "0");
    strcpy(pcb->registros->EAX, "0");
    strcpy(pcb->registros->EBX, "0");
    strcpy(pcb->registros->ECX, "0");
	strcpy(pcb->registros->EDX, "0");
	strcpy(pcb->registros->RAX, "0");
	strcpy(pcb->registros->RBX, "0");
	strcpy(pcb->registros->RCX, "0");
	strcpy(pcb->registros->RDX, "0");

    return pcb;
}

t_socket crearConexionCPU(){
	char* ip = config_get_string_value(config,"IP_CPU");
	char* puerto = config_get_string_value(config,"PUERTO_CPU");
	t_socket conexion = crear_conexion(ip, puerto, logger);
	return conexion;
}


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

	while(1);
}


void inicializarRecursos(){
	char* recursos_config =  config_get_string_value(config, "RECURSOS");
	// TODO: alejiti fijate q la cantidad es un vector
	cantidad_recursos = contar(recursos_config, ',') + 1; //cuento las comas y le sumo uno para saber cantidad de recursos
	// Falta un if para ver si no hay recursos!!

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

void inicializarSemoforos(){
	sem_init(&sem_new_a_ready, 0, 0); //Binario, iniciado en 0 para que solo agregue a ready si hay procesos en new
	sem_init(&sem_ready, 0, 0); //Binario, solo pase a CPU si hay en ready
	sem_init(&sem_grado_multiprogramacion, 0, config_get_int_value(config,"GRADO_MAX_MULTIPROGRAMACION"));
}


void liberarSemoforos(){
	sem_destroy(&sem_new_a_ready);
	sem_destroy(&sem_ready);
	sem_destroy(&sem_grado_multiprogramacion);
	liberarMutex();
}

void liberarMutex(){ //Semaforos mutex para acceder a las listas de procesos
	pthread_mutex_destroy(&mutex_procesos_ready);
	pthread_mutex_destroy(&mutex_procesos_new);
	pthread_mutex_destroy(&mutex_procesos_execute);
}

t_contexto* actualizar_pcb(t_pcb* proceso) {
	if(conexionCPU != -1){
				t_contexto* contexto;
				int cod_op = recibir_operacion(conexionCPU);
				if(cod_op == CONTEXTO) {
					contexto = recibir_contexto(conexionCPU);
					// actualizo proceso con lo q viene del pcb (PC y registros)
					log_info(logger, "Recibo contexto pa - PID:%d\n", contexto->pid);

					proceso->pid = contexto->pid;
					proceso->pc = contexto->pc;
					proceso->motivo = contexto->motivo;
					strcpy(proceso->registros->AX, contexto->registros->AX);
					strcpy(proceso->registros->BX, contexto->registros->BX);
					strcpy(proceso->registros->CX, contexto->registros->CX);
					strcpy(proceso->registros->DX, contexto->registros->DX);
					strcpy(proceso->registros->EAX, contexto->registros->EAX);
					strcpy(proceso->registros->EBX, contexto->registros->EBX);
					strcpy(proceso->registros->ECX, contexto->registros->ECX);
					strcpy(proceso->registros->EDX, contexto->registros->EDX);
					strcpy(proceso->registros->RAX, contexto->registros->RAX);
					strcpy(proceso->registros->RBX, contexto->registros->RBX);
					strcpy(proceso->registros->RCX, contexto->registros->RCX);
					strcpy(proceso->registros->RDX, contexto->registros->RDX);


				} else {
					log_error(logger,"No me llego un proceso");
				}
			return contexto;
		}

}

void ejecutarIO(int tiempo) {
	pthread_t hilo_bloqueoPorIO;
	printf("%d\n",tiempo);
	pthread_create(&hilo_bloqueoPorIO,NULL,(void*)bloquearYPasarAReady,tiempo);
	pthread_detach(hilo_bloqueoPorIO);
}

void bloquearYPasarAReady(int tiempo) {
	t_pcb* proceso = sacarDeCPU();
	log_info(logger,"Se bloqueara el Proceso %d por IO durante %d segundos", proceso->pid,tiempo);
	sleep(tiempo);
	log_info(logger,"Finaliza bloqueo de Proceso %d por IO y pasa a estado ready", proceso->pid);
	pthread_mutex_lock(&mutex_procesos_ready);
	list_add(procesosReady, proceso);
	pthread_mutex_unlock(&mutex_procesos_ready);
	sem_post(&sem_ready);
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

void wait(char* recurso) {

    disminuirInstancias(recurso);

    if(cantInstancias(recurso) < 0) {
        bloquear(recurso);
    } else {
    t_pcb* proceso = sacarDeCPU();
    agregarAlInicioDeReady(proceso);
    log_info(logger, "Proceso %d vuelve a cpu por disponibilidad del recurso %s\n", proceso->pid, recurso);
    }
}


void ejecutarSignal(char* recurso) {
    aumentarInstancias(recurso);

    if(cantInstancias(recurso) <= 0) {
         desbloquearPrimerProceso(recurso);
    }
    t_pcb* proceso = sacarDeCPU();
	agregarAlInicioDeReady(proceso);
	log_info(logger, "Se realizo signal del recurso %s. El proceso %d puede seguir ejecutandose en cpu\n", recurso, proceso->pid);

}

int indice(char* recurso) {

    for(int i=0; i < cantidad_recursos; i++) {
        if(strcmp(recursos[i], recurso) == 0) {
          return i;
        }
    }
}

int cantInstancias(char* recurso) {
	int i =  indice(recurso);

	return instancias[i];
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
    pthread_mutex_lock(&mutex_procesos_ready);
	list_add(procesosReady, proceso);
	pthread_mutex_unlock(&mutex_procesos_ready);
    log_info(logger, "Se desbloqueo el proceso %d de la cola del recurso %s",proceso->pid, recurso);
    queue_pop(cola);

}

void bloquear(char* recurso) {
    int i = indice(recurso);
    t_queue* cola = list_get(colasDeBloqueados, i);
    t_pcb* proceso = sacarDeCPU();
    queue_push(cola, proceso);
    log_info(logger, "El proceso %d queda bloqueado por falta de instancias del recurso %s", proceso->pid, recurso);

}

t_pcb* sacarDeCPU() {
    pthread_mutex_lock(&mutex_procesos_execute);
	t_pcb* proceso = list_remove(procesosExecute, 0);
	pthread_mutex_unlock(&mutex_procesos_execute);
	printf("Proceso que sacamos de execute:%d\n", proceso->pid);
	return proceso;
}

void agregarAlInicioDeReady(t_pcb* proceso) {
	pthread_mutex_lock(&mutex_procesos_ready);
	list_add_in_index(procesosReady,0,proceso);
	pthread_mutex_unlock(&mutex_procesos_ready);
	sem_post(&sem_ready);
}

void pasarAInstanciasEnteras() {
	for(int i=0; i < cantidad_recursos; i++) {
        instancias[i] = atoi(instanciasRecursos[i]);
    }
}


