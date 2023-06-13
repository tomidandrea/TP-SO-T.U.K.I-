#include <memoria.h>
#include <sincro.h>

t_log* logger;
t_config* config;
t_socket server_fd;
void* espacioMemoria;
extern tabla_segmentos* tablaSegmentos;

int main(int argc, char* argv[]) {
	logger = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);
	server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");
	log_debug(logger, "Iniciando memoria");
	crearEscucharKernel();
	crearEscucharCPU();
	crearEscucharFS();

	while(1);

}

