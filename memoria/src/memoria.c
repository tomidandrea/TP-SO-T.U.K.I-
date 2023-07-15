#include <memoria.h>

t_log* logger;
t_config* config;
t_socket server_fd;
void* espacioMemoria;
tabla_segmentos tabla_huecos;
sem_t sem_cpu, ejecutando, sem_finalizar_hilo_main;
int retardo_memoria;

int main(int argc, char* argv[]) {
	iniciarConexionMemoria(argv[1]);

	log_info(logger, "Inicializando estructuras...");
	inicializarEstructuras();

	sem_init(&sem_cpu, 0, 0);
	sem_init(&ejecutando, 0, 1);
	sem_init(&sem_finalizar_hilo_main, 0, 0);

	retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");

	log_debug(logger, "Iniciando hilos memoria");

	crearEscucharCPU();
	crearEscucharKernel();
	crearEscucharFS();

	//while(1);
	sem_wait(&sem_finalizar_hilo_main);
	sem_wait(&sem_finalizar_hilo_main);
	sem_wait(&sem_finalizar_hilo_main);

	sem_destroy(&sem_cpu);
	sem_destroy(&ejecutando);
	sem_destroy(&sem_finalizar_hilo_main);
	return 0;
}

void iniciarConexionMemoria(char* path){
	logger = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(path);
	log_debug(logger, "Conexion iniciada");
	server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");

}

