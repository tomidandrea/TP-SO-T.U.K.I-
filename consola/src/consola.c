#include <consola.h>
t_log* logger;
t_config* config;

int main(int argc, char** argv) {
    t_socket conexion;
    char* ip;
    char* puerto;
    char* path_config;
    char* path_inst;

    logger = iniciar_logger("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);
    path_config = copiar(argv[1]);
    config = iniciar_config(path_config);
    free(path_config);

    log_info(logger, "Inicio consola \n");
    path_inst = copiar(argv[2]); //asigno el path de pseudo

    t_list* instrucciones = list_create();
    parsear_instrucciones(path_inst, instrucciones); //me devuelve la lista con las instrucciones
	free(path_inst);

    logearInstrucciones(instrucciones, logger); //logeamos la lista de instrucciones

    ip = config_get_string_value(config,"IP_KERNEL");
    puerto = config_get_string_value(config,"PUERTO_KERNEL");
    conexion = crear_conexion(ip, puerto, logger);


    // serializa y envia a kernel el programa (lista de instrucciones)
    enviar_programa(instrucciones, conexion);
    printf("\nsocket conexion:%d \n",conexion);
    liberar_instrucciones(instrucciones);
    list_destroy(instrucciones);


    uint32_t result;
    recv(conexion, &result, sizeof(uint32_t), MSG_WAITALL);
    if(result == 0){
    	log_info(logger, "Resultado: Termino todo bien pa");
    }else
    	log_info(logger, "Resultado: Rompiste algo");

    liberar_conexion(conexion); // Libera ip y puerto
    liberarEstructuras();
    return 0;
}

void liberarEstructuras(){
	log_destroy(logger);
	config_destroy(config);
}

