#include <consola.h>
t_log* logger;
t_config* config;

int main(int argc, char** argv) {
    t_socket conexionKernel;
    char* path_config;
    char* path_inst;

    logger = iniciar_logger("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);
    path_config = copiar(argv[1]);
    config = iniciar_config(path_config);
    free(path_config);

    log_info(logger, "Inicio consola");
    path_inst = copiar(argv[2]); //asigno el path de pseudo

    t_list* instrucciones = list_create();
    parsear_instrucciones(path_inst, instrucciones); //me devuelve la lista con las instrucciones
	free(path_inst);

    logearInstrucciones(instrucciones, logger); //logeamos la lista de instrucciones

    conexionKernel = iniciarConexion(config, logger, "IP_KERNEL","PUERTO_KERNEL");
    log_info(logger, "Enviando programa");
    // serializa y envia a kernel el programa (lista de instrucciones)
    enviar_programa(instrucciones, conexionKernel);
    liberar_instrucciones(instrucciones);
    list_destroy(instrucciones);

    uint32_t result;
	if(recv(conexionKernel, &result, sizeof(uint32_t), MSG_WAITALL)> 0){
		if(result == 0){
				log_info(logger, "Resultado: Termino todo bien pa");
			}else
				log_info(logger, "Resultado: Rompiste algo");

			liberarEstructuras(conexionKernel);
	}else{
		log_error(logger,"Se cerró kernel");
		liberarEstructuras(conexionKernel);
		exit(1);
	}

    return 0;
}

void liberarEstructuras(t_socket conexion){
	liberar_conexion(conexion); // Libera ip y puerto
	log_destroy(logger);
	config_destroy(config);
}

