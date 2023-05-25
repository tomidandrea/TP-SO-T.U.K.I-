#include <utils.h>
#include <comunicacion.h>

int processID=0;
extern t_config* config;
extern t_log* logger;
extern t_socket conexionCPU;

extern char** recursos;
extern char** instanciasRecursos;
extern int cantidad_recursos;

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
    pcb->estimadoAnterior = config_get_double_value(config,"ESTIMACION_INICIAL");

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
//	for (int i = 0; i < cantidad_recursos; ++i) {
//		log_info(logger, "Recurso %d: %s tiene %s instancia/s", i, recursos[i], instanciasRecursos[i]);
//	}
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

					temporal_stop(proceso->tiempoCPU);
					int64_t tiempo = temporal_gettime(proceso->tiempoCPU);
					//todo: reiniciar tiempo
					// actualizo proceso con lo q viene del pcb (PC y registros)
					log_info(logger, "Recibo contexto pa - PID:%d\n", contexto->pid);

					proceso->pid = contexto->pid;
					proceso->pc = contexto->pc;
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

					liberar_contexto(contexto);

				} else {
					log_error(logger,"No me llego un proceso");
				}
			return contexto;
		}

}

void iniciarTiempoEnReady(t_pcb* proceso){
	t_temporal* tiempo;
	tiempo = temporal_create();
	proceso->tiempoEnReady = tiempo;
}

void iniciarTiempoCPU(t_pcb* proceso){
	t_temporal* tiempo;
		tiempo = temporal_create();
		proceso->tiempoCPU = tiempo;
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



