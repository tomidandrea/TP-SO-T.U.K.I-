#include <memoria.h>

t_log* logger;
t_config* config;
t_socket server_fd;
void* espacioMemoria;
tabla_segmentos tabla_huecos;
sem_t sem_cpu, sem_kernel;


int main(int argc, char* argv[]) {
	iniciarConexionMemoria(argv[1]);

	log_info(logger, "Inicializando estructuras...");
	inicializarEstructuras();

	sem_init(&sem_cpu, 0, 0);
	sem_init(&sem_kernel, 0, 0);

	log_debug(logger, "Iniciando hilos memoria");
	crearEscucharCPU();
	crearEscucharKernel();
	//crearEscucharFS();

	while(1);

}

void iniciarConexionMemoria(char* path){
	logger = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(path);
	log_debug(logger, "Conexion iniciada");
	server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");

}

