#include <consola.h>

int main(int argc, char** argv[]) {
    t_log* logger;
    t_config* config;
    logger = iniciar_logger();
    config = iniciar_config(argv[1]);
    log_info(logger, "Inicio consola \n");


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
