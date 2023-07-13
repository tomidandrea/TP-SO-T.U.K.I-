#include <kernel.h>


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

//todo: mallocs y free para todos los punteros

int main(int argc, char* argv[]) {
    logger = iniciar_logger("kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    config = iniciar_config(argv[1]);
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

		crearEscucharConsolas();
		crearAgregarReady();
		crearPlanificar();
		crearRecibirDeCPU();
		crearRecibirDeFS();
		while(1);

		return EXIT_SUCCESS;
}



