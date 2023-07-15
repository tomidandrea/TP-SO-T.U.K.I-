#include <kernel.h>
#include <signal.h>


t_log* logger;
t_config* config;
t_list* procesosExecute;
t_list* procesosReady;
t_list* procesosNew;
t_list* colasDeBloqueados;
t_list* archivosAbiertosGlobal;
t_list* esperaDeFS;
t_list* esperaDeIO;
t_socket conexionCPU;
t_socket conexionMemoria;
t_socket conexionFileSystem;
char** recursos;
char** instanciasRecursos;
int* instancias;

sem_t sem_finalizar;
extern sem_t sem_new_a_ready, sem_ready, sem_recibir_cpu, sem_recibir_fs;
extern t_socket socket_cliente;

t_list* listaProcesosGlobal;
bool seguir_ejecucion = true;

int main(int argc, char* argv[]) {
    logger = iniciar_logger("kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    config = iniciar_config(argv[1]);

    listaProcesosGlobal = list_create();

    inicializarRecursos();

    colasDeBloqueados = list_create();
    crearColasDeBloqueados();
    //no hay que verificar acá, solo de prueba
    /*char* recursoEjemplo = "DISCO";
    verificarRecursos(recursoEjemplo);*/
    inicializarSemoforos();

    archivosAbiertosGlobal = list_create();

    procesosNew = list_create();
    procesosReady = list_create();
    procesosExecute = list_create();

    esperaDeIO = list_create();
    esperaDeFS = list_create();

    conexionCPU = iniciarConexion(config, logger, "IP_CPU", "PUERTO_CPU");
    conexionMemoria = iniciarConexion(config, logger, "IP_MEMORIA", "PUERTO_MEMORIA");
    conexionFileSystem = iniciarConexion(config, logger, "IP_FILESYSTEM", "PUERTO_FILESYSTEM");

    signal(SIGINT, cerrar);

	crearEscucharConsolas();
	crearAgregarReady();
	crearPlanificar();
	crearRecibirDeCPU();
	crearRecibirDeFS();
	while(seguir_ejecucion);
	printf("\nHola termine\n");
	return EXIT_SUCCESS;
}

void cerrar(int signal){
	printf("\nHola te libero algunas estructuras\n");
	seguir_ejecucion = false;
	close(conexionCPU);
	close(conexionFileSystem);
	close(conexionMemoria);
	close(socket_cliente);
	list_destroy_and_destroy_elements(listaProcesosGlobal, liberar_pcb);

	sem_post(&sem_new_a_ready);
	sem_post(&sem_ready);
	sem_post(&sem_recibir_cpu);
	sem_post(&sem_recibir_fs);

	sem_wait(&sem_finalizar);
	liberarSemoforos();
	usleep(10000000);
	exit(1);
}


