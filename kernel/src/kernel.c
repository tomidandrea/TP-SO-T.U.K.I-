#include <kernel.h>


t_log* logger;
t_config* config;
t_list* procesosExecute;
t_list* procesosReady;
t_list* procesosNew;

//todo: mallocs y free para todos los punteros


int main(int argc, char* argv[]) {
    logger = iniciar_logger("kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    config = iniciar_config(argv[1]);
    inicializarSemoforos();

    procesosNew = list_create();
    procesosReady = list_create();
    procesosExecute = list_create();

	// Hilo que atiende consolas

		crearEscucharConsolas();
		crearAgregarReady();
		crearPlanificar();
		while(1);

		return EXIT_SUCCESS;
}

void mandarCpu(){
	char* ip = config_get_string_value(config,"IP_CPU");
	char* puerto = config_get_string_value(config,"PUERTO_CPU");
	t_socket conexion = crear_conexion(ip, puerto, logger);
}


