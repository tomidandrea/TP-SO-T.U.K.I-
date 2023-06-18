#include <memoria.h>
#include <sincro.h>

t_log* logger;
t_config* config;
t_socket server_fd;
void* espacioMemoria;
sem_t sem_cpu, sem_kernel;
int retardo_memoria;

int main(int argc, char* argv[]) {
	sem_init(&sem_cpu, 0, 0);
	sem_init(&sem_kernel, 0, 0);
	logger = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);
	server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");
	log_debug(logger, "Iniciando memoria");

	retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
	crearEscucharCPU();
	crearEscucharKernel();
	//crearEscucharFS();

	while(1);

}

