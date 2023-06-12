#include <file-system.h>

t_log* logger;
t_config* config;
t_socket conexionMemoria;

int main(int argc, char* argv[]) {

	size_t cantidad_bloques, cantidad_bytes;
	int result_map;

	logger = iniciar_logger("file-system.log", "FILE SYSTEM", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);

	//inicio conexion con memoria
	conexionMemoria = iniciarConexion(config, logger, "IP_MEMORIA","PUERTO_MEMORIA");

	//levanto archivo superbloque (lo trato como config ya que es compatible)

	char* path_superbloque = config_get_string_value(config,"PATH_SUPERBLOQUE");

	t_config* superbloque = iniciar_config(path_superbloque);

	free(path_superbloque);

	//creo bitmap en memoria
	cantidad_bloques = config_get_int_value(superbloque,"BLOCK_COUNT");

	cantidad_bytes = cantidad_bloques/8;                                 //cantidad bloques = cantidad bits

	void *puntero_a_bits = malloc(cantidad_bytes);

	t_bitarray* bitmap = bitarray_create_with_mode(puntero_a_bits, cantidad_bytes, LSB_FIRST);

	//mapeo archvivo de bitmap al bitmap creado en memoria

	char* path_bitmap = config_get_string_value(config,"PATH_BITMAP");

	 printf("empiezo a mapear bitmap\n");

     result_map = mapearArchivo(bitmap->bitarray,path_bitmap,cantidad_bytes);
     if (result_map == 0) {
    	 log_error(logger,"error al mapear archivo de bitbloques");
    	 exit(1);
     }

     //creo array de bloques en memoria

     int tamanio_bloque = config_get_int_value(superbloque,"BLOCK_SIZE");

     int tamanio_total = cantidad_bloques*tamanio_bloque;

     void*bloques = malloc(tamanio_total);


     //mapeo archivo de bloques con el array de bloques
     char* path_bloques = config_get_string_value(config,"PATH_BLOQUES");

     printf("empiezo a mapear bloques\n");
     result_map = mapearArchivo(bloques,path_bloques,tamanio_total);
         if (result_map == 0) {
        	 log_error(logger,"error al mapear archivo de bloques");
        	 exit(1);
         }


    //inicio servidor para kernel
	t_socket server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");

	while(1){
		t_socket socket_cliente = esperar_cliente(server_fd, logger);
	}

	return EXIT_SUCCESS;
}
