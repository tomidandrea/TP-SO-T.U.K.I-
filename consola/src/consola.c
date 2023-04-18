#include <consola.h>

void logearInstrucciones(t_list* instrucciones, t_log* logger){
	int cant = list_size(instrucciones);
	        for(int i = 0;i<cant;i++) {
	    	t_instruccion* inst = list_get(instrucciones,i);

	        log_info(logger, "Instruccion: %s", inst->instruccion);

	        int cant_parametros = cantParametros(inst->instruccion);

	    	for(int i=0; i<cant_parametros; i++) {
	    		log_info(logger, "Parametro %d: %s", i, inst->parametros[i]);
	    	}
	        printf("------------\n");
	    }
}

int main(int argc, char** argv) {

    t_log* logger;
    t_config* config;
    t_socket conexion;
    char* ip;
    char* puerto;
    char* path;

    logger = iniciar_logger();
    config = iniciar_config(argv[1]);

    log_info(logger, "Inicio consola \n");
    //asigno el path de pseudo
    path = string_duplicate(argv[2]);

    t_list* instrucciones = list_create();
    parsear_instrucciones(path, instrucciones); //me devuelve la lista con las instrucciones
    free(path);

    //logeamos la lista de instrucciones
    logearInstrucciones(instrucciones, logger);


    ip = config_get_string_value(config,"IP_KERNEL");
    puerto = config_get_string_value(config,"PUERTO_KERNEL");
    conexion = crear_conexion(ip, puerto, logger);

    // serializa y envia a kernel el programa (lista de instrucciones)
    enviar_programa(instrucciones, conexion);

    return 0;
}

t_config* iniciar_config (char* path){
	t_config* nuevo_config = config_create(path);
	if(nuevo_config== NULL){
		printf("No se pudo crear el config\n");
	}

	return nuevo_config;
}

t_log* iniciar_logger(void)
{
	char* file = "consola.log";
	char *process_name = "CONSOLA";
	bool is_active_console = true;
	t_log_level level = LOG_LEVEL_INFO;
	t_log* nuevo_logger = log_create(file, process_name,
			is_active_console, level);

	return nuevo_logger;
}
