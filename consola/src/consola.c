#include <consola.h>
#include <utils/sockets.h>

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

    //verifico por pantalla las instrucciones de la lista
    int cant = list_size(instrucciones);
    printf("Cantidad de instrucciones: %d\n", cant);

    for(int i = 0;i<cant;i++) {
    	t_instruccion* inst = list_get(instrucciones,i);
    	printf("%s \n", inst->instruccion);
    	int cant_parametros = cantParametros(inst->instruccion);
    	printf("Cantidad de param: %d\n", cant_parametros);

    	for(int i=0; i<cant_parametros; i++) {
    		printf("Parametro %d: %s \n", i, inst->parametros[i]);
    	}
    }

    ip = config_get_string_value(config,"IP_KERNEL");
    puerto = config_get_string_value(config,"PUERTO_KERNEL");
    conexion = crear_conexion(ip, puerto, logger);

    // serializo la lista de instrucciones

    serializar_programa(instrucciones);


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
