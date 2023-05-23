#include <kernel.h>


t_log* logger;
t_config* config;
t_list* procesosExecute;
t_list* procesosReady;
t_list* procesosNew;
t_socket conexionCPU;

//todo: mallocs y free para todos los punteros




int main(int argc, char* argv[]) {
    logger = iniciar_logger("kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    config = iniciar_config(argv[1]);
    verificarRecursos(config);
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

int verificarRecursos(t_list* recursosContexto,t_list* instanciasDeRecursos, t_config* config){
	t_list* recursos = list_create();
	t_list* instanciasRecursosConfig = list_create();
	recursos = config_get_array_value(config, "RECURSOS");
	instanciasRecursosConfig = config_get_array_value(config, "INSTANCIAS_RECURSOS");

	int i = 0;
	int a = 0;
	//TODO JARWI VER SI HAY ALGUNA DE FORMA DE RECORRER LA LISTA MAS FACIL
	for(i=0; i <= length(recursosContexto);i++){
		for(a=0; i <= length(recursos);a++){
			if(list_get(recursosContexto, i) = list_get(recursos, a)){
				//ahora chequeo los recursos a ver que onda
				if(list_get(instanciasDeRecursos, i) <= list_get(instanciasRecursosConfig, a)){
					//TODO JARWI VER QUE DEVOLVER ACA
					//
				}

			}
		}
	}

}

