#include <consola.h>
t_log* logger;
t_config* config;

int main(int argc, char** argv) {
    logger = malloc(sizeof(t_log));
    config = malloc(sizeof(t_config));
    t_socket conexion;
    char* ip = malloc(TAMANIO_IP);
    char* puerto = malloc(TAMANIO_PUERTO);
    char* path = malloc(TAMANIO_PATH);

    logger = iniciar_logger("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);
    config = iniciar_config(argv[1]);

    log_info(logger, "Inicio consola \n");
    path = string_duplicate(argv[2]); //asigno el path de pseudo

    t_list* instrucciones = list_create();
    parsear_instrucciones(path, instrucciones); //me devuelve la lista con las instrucciones
	free(path);

    logearInstrucciones(instrucciones, logger); //logeamos la lista de instrucciones

    ip = config_get_string_value(config,"IP_KERNEL");
    puerto = config_get_string_value(config,"PUERTO_KERNEL");
    conexion = crear_conexion(ip, puerto, logger);
	free(ip);
	free(puerto);

    // serializa y envia a kernel el programa (lista de instrucciones)
    enviar_programa(instrucciones, conexion);
    printf("\nsocket conexion:%d \n",conexion);
    list_destroy(instrucciones);


    //uint32_t result = malloc(sizeof(uint32_t));
    uint32_t result;
    /*recv(conexion, &result, sizeof(uint32_t), MSG_WAITALL);
    if(result == 0){
    	log_info(logger, "Resultado: Termino todo bien pa");
    }else
    	log_info(logger, "Resultado: Rompiste algo");
*/
    liberarEstructuras();
    return 0;
}

void liberarEstructuras(){
	log_destroy(logger);
	config_destroy(config);
}

