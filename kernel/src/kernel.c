#include <kernel.h>


t_log* logger;
t_config* config;
t_list* procesosExecute;
t_list* procesosReady;
t_list* procesosNew;
t_list* colasDeBloqueados;
t_socket conexionCPU;
char** recursos;
char** instanciasRecursos;
int cantidad_recursos;
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

    procesosNew = list_create();
    procesosReady = list_create();
    procesosExecute = list_create();

    conexionCPU = crearConexionCPU();
	// Hilo que atiende consolas

		crearEscucharConsolas();
		crearAgregarReady();
		crearPlanificar();
		while(1);

		return EXIT_SUCCESS;
}



