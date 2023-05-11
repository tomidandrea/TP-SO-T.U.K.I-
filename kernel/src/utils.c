#include <utils.h>
#include <comunicacion.h>

int processID=0;
extern t_config* config;
extern t_log* logger;


extern sem_t sem_new_a_ready, sem_ready, sem_grado_multiprogramacion;
extern pthread_mutex_t mutex_procesos_new;
extern pthread_mutex_t mutex_procesos_ready;
extern pthread_mutex_t mutex_procesos_execute;

t_pcb* crearPCB(t_list* listaInstrucciones){

	t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->pid = ++processID;
    pcb->pc = 0;
    pcb->estado_ejec = (estado_ejec) NUEVO;
    pcb->instrucciones = list_duplicate(listaInstrucciones);
    pcb->registros = inicializarRegistros();
    strcpy(pcb->registros->AX, "0");
    strcpy(pcb->registros->BX, "0");
    strcpy(pcb->registros->CX, "0");
    strcpy(pcb->registros->DX, "0");
    strcpy(pcb->registros->EAX, "TEXTO111");
    strcpy(pcb->registros->EBX, "0");
    strcpy(pcb->registros->ECX, "0");
	strcpy(pcb->registros->EDX, "0");
	strcpy(pcb->registros->RAX, "0");
	strcpy(pcb->registros->RBX, "0");
	strcpy(pcb->registros->RCX, "0");
	strcpy(pcb->registros->RDX, "0");

    return pcb;
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

