#include "utilsKernel.h"

#include <comunicacion.h>

int processID=0;
uint32_t BLOQUEADO = 0;
uint32_t CONTINUAR_EJECUTANDO = 1;
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

extern sem_t sem_new_a_ready, sem_ready, sem_grado_multiprogramacion, sem_recibir, sem_execute;
extern pthread_mutex_t mutex_procesos_new;
extern pthread_mutex_t mutex_procesos_ready;
extern pthread_mutex_t mutex_procesos_execute;

t_socket crearConexionCPU(){
	char* ip = config_get_string_value(config,"IP_CPU");
	char* puerto = config_get_string_value(config,"PUERTO_CPU");
	t_socket conexion = crear_conexion(ip, puerto, logger);
	return conexion;
}

t_socket crearConexionMemoria(){
	char* ip = config_get_string_value(config,"IP_MEMORIA");
	char* puerto = config_get_string_value(config,"PUERTO_MEMORIA");
	t_socket conexion = crear_conexion(ip, puerto, logger);
	return conexion;
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

t_pcb* crearPCB(t_list* listaInstrucciones, t_socket socket_consola){

	t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->pid = ++processID;
    pcb->pc = 0;
    pcb->socket_consola = socket_consola;
    pcb->instrucciones = list_duplicate(listaInstrucciones);
    pcb->registros = inicializarRegistros();
    pcb->estimadoAnterior = config_get_double_value(config,"ESTIMACION_INICIAL");
    pcb->tiempoEnReady = iniciarTiempo();
    temporal_stop(pcb->tiempoEnReady);
    pcb->tiempoCPU = iniciarTiempo();
    temporal_stop(pcb->tiempoCPU);

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

t_contexto* actualizar_pcb(t_pcb* proceso) {
	if(conexionCPU != -1){
				t_contexto* contexto;
				int cod_op = recibir_operacion(conexionCPU);
				if(cod_op == CONTEXTO) {

					contexto = recibir_contexto(conexionCPU);

					temporal_stop(proceso->tiempoCPU);
					// actualizo proceso con lo q viene del pcb (PC y registros)
					log_info(logger, "Recibo contexto pa - PID:%d", contexto->pid);

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

t_temporal* iniciarTiempo(){
	t_temporal* tiempo;
	tiempo = temporal_create();
	return tiempo;
}

t_temporal* pararTiempo(t_temporal* temporal){
	temporal_stop(temporal);
	temporal_destroy(temporal);
	t_temporal* nuevoTemporal;
	nuevoTemporal = temporal_create();
	temporal_stop(temporal);
	return nuevoTemporal;
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


void pasarAInstanciasEnteras() {
	for(int i=0; i < cantidad_recursos; i++) {
        instancias[i] = atoi(instanciasRecursos[i]);
    }
}

io_contexto* inicializarIoContexto(t_pcb* proceso, int tiempo){
	io_contexto* contexto = malloc(sizeof(io_contexto));
	contexto->proceso = proceso;
	contexto->tiempo_sleep = tiempo;
	return contexto;
}

