#include <file-system.h>

t_log* logger;
t_config* config;
t_socket conexionMemoria;

uint32_t RESULT_OK = 1;
uint32_t RESULT_ERROR = 0;


int main(int argc, char* argv[]) {

	size_t cantidad_bloques, cantidad_bytes;
	t_list* fcbs = list_create();
	int result_map, result_operacion;

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
    	 log_error(logger,"error al mapear archivo de bitmap");
    	 exit(1);
     }

     free(path_bitmap);

     //creo array de bloques en memoria

     int tamanio_bloque = config_get_int_value(superbloque,"BLOCK_SIZE");

     int tamanio_total = cantidad_bloques*tamanio_bloque;

     void*bloques = malloc(tamanio_total);


     //mapeo archivo de bloques con el array de bloques
     char* path_bloques = config_get_string_value(config,"PATH_BLOQUES");

     printf("empiezo a mapear bloques\n");
     result_map = mapearArchivo(bloques,path_bloques,tamanio_total);
         if (result_map == RESULT_ERROR) {
        	 log_error(logger,"error al mapear archivo de bloques");
        	 exit(1);
         }

     free(path_bloques);

    //inicio servidor para kernel
	t_socket server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");

	t_socket socket_cliente = esperar_cliente(server_fd, logger);

	while(1) {
		int cod_op = recibir_operacion(socket_cliente);
		char** parametros = string_array_new();
		recibo_parametros(socket_cliente,parametros);

		switch (cod_op) {

		        case F_CREATE:
					 log_info(logger, "Crear Archivo: %s ", parametros[0]);
					 crear_fcb(parametros[0],fcbs);
		        	 result_operacion = RESULT_OK;
					 break;

				case F_OPEN:
					 log_info(logger, "Abrir Archivo: %s ", parametros[0]);
					 result_operacion = existe_fcb(parametros[0],fcbs);
					 break;

				case F_TRUNCATE:
					// log_info(logger, "Truncar Archivo: %s - Tamaño: %s", parametros[0], parametros[1]);
					// result_operacion = truncar_archivo(parametro[0],parametro[1]);
					break;

				case F_READ:
				    //result_operacion = leer_archivo(parametros[0],parametros[1],parametros[2]);
				    break;

				case F_WRITE:
					//result_operacion = escribir_archivo(parametros[0],parametros[1],parametros[2]);
					break;

		 string_array_destroy(parametros);
		}

		send(socket_cliente, (void *)(intptr_t)result_operacion, sizeof(uint32_t), (intptr_t)NULL);
	}

	return EXIT_SUCCESS;
}




void crear_fcb(char* archivo, t_list*fcbs) {

	t_fcb* fcb;
	strcpy(fcb->nombre,archivo);
	fcb->tamanio = 0;
	fcb->puntero_directo = NULL;
	fcb->puntero_indirecto = NULL;

	list_add(fcbs,fcb);

	//TODO ver como y cuando persistirlo en disco
}


int existe_fcb(char*archivo, t_list*fcbs) {

	int i;
	int tamanio = list_size(fcbs);

	for(i=0; i<tamanio; i++) {
		t_fcb*fcb = list_get(fcbs,i);

		if (strcmp(archivo,fcb->nombre) == 0)
			return 1;
	}
	return 0;

// ver sino list_any_satisfy() de las commons;
}


