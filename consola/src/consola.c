#include <consola.h>

int main(int argc, char** argv) {
    t_log* logger;
    t_config* config;
    t_socket conexion;
    char* ip;
    char* puerto;
    char* path;

    logger = iniciar_logger("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);
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

    int result;
    recv(conexion, &result, sizeof(int), MSG_WAITALL);
    if(result == 0){
    	log_info(logger, "Resultado: Termino todo bien pa");
    }else
    	log_info(logger, "Resultado: Rompiste algo");

    return 0;
}

