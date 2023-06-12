#include <memoria.h>
#include <sincro.h>

t_log* logger;
t_config* config;
void* espacioMemoria;
extern tabla_segmentos* tablaSegmentos;

int main(int argc, char* argv[]) {
	logger = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);

	crearEscucharKernel();
	crearEscucharCPU();
	crearEscucharFS();



}

