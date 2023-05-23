#include <kernel.h>


t_log* logger;
t_config* config;
t_list* procesosExecute;
t_list* procesosReady;
t_list* procesosNew;
t_socket conexionCPU;
char** recursos;
char** instanciasRecursos;
int cantidad_recursos;

//todo: mallocs y free para todos los punteros

int main(int argc, char* argv[]) {
    logger = iniciar_logger("kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    config = iniciar_config(argv[1]);
    inicializarRecursos();
    //no hay que verificar acá, solo de prueba
    char* recursoEjemplo = "DISCO";
    verificarRecursos(recursoEjemplo);
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



