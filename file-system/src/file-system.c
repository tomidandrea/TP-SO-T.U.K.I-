#include <file-system.h>

t_log* logger;
t_config* config;
t_socket conexionMemoria;

bool RESULT_OK = 1;
bool RESULT_ERROR = 0;


int main(int argc, char* argv[]) {

	size_t cantidad_bloques, cantidad_bytes;
	t_list* fcbs = list_create();
	bool result_operacion;

	logger = iniciar_logger("file-system.log", "FILE SYSTEM", true, LOG_LEVEL_DEBUG);
	config = iniciar_config(argv[1]);

	//inicio conexion con memoria
	conexionMemoria = iniciarConexion(config, logger, "IP_MEMORIA","PUERTO_MEMORIA");

	//levanto archivo superbloque (lo trato como config ya que es compatible)
	char* path_superbloque = config_get_string_value(config,"PATH_SUPERBLOQUE");
	t_config* superbloque = iniciar_config(path_superbloque);
	free(path_superbloque);

	//obtengo la cantidad de bloques/bits del archivo de superbloque y calculo la cantidad de bytes que equivale
	cantidad_bloques = config_get_int_value(superbloque,"BLOCK_COUNT");
	cantidad_bytes = ceil(cantidad_bloques/8);                        //si da con coma redondea al proximo entero

	//levanto el archivo bitmap (si no esta creado, lo creo
	char* path_bitmap = config_get_string_value(config,"PATH_BITMAP");
	FILE* archivo_bitmap = levantarArchivo(path_bitmap,cantidad_bloques);
	free(path_bitmap);

	//mapeo el archivo bitmap en un bitarray
	t_bitarray* bitmap = mapear_bitmap(cantidad_bloques,cantidad_bytes,archivo_bitmap);  // devuelve bitarray creado despues de mapear

     //levanto archivo de bloques (si no existe lo creo)

	 char* path_bloques = config_get_string_value(config,"PATH_BLOQUES");
     int tamanio_bloque = config_get_int_value(superbloque,"BLOCK_SIZE");
     int tamanio_total = cantidad_bloques*tamanio_bloque;

     FILE*archivo_bloques = levantarArchivo(path_bloques,tamanio_total);
     free(path_bloques);
     //TODO ver si es mejor directamente usar fseek,fread y frwite (no mapearlo en memoria)
     /*
     //mapeo archivo de bloques en un espacio de memoria
     void*bloques = mapearArchivo(archivo_bloques,tamanio_total);
     */

    //creo un fcb de prueba
     char* path_fcbs = config_get_string_value(config,"PATH_FCB");

     crear_fcb("/pruebafcb",fcbs,path_fcbs);


    //inicio servidor para kernel
	t_socket server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");

	t_socket socket_cliente = esperar_cliente(server_fd, logger);

	while(1) {
		int cod_op = recibir_operacion(socket_cliente);
		char** parametros = string_array_new();
		recibo_parametros(socket_cliente,parametros);

		switch (cod_op) {

		        case CREAR_ARCHIVO:
					 log_info(logger, "Crear Archivo: %s ", parametros[0]);
					 crear_fcb(parametros[0],fcbs,path_fcbs);
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

	free(path_fcbs);

	return EXIT_SUCCESS;
}


void crear_fcb(char* archivo, t_list*fcbs, char*path) {

	printf("creando fcb del archivo %s\n", archivo);

	t_fcb* fcb = malloc(sizeof(t_fcb));
	fcb->nombre = string_duplicate(archivo);
	fcb->tamanio = 0;
	fcb->puntero_directo = 0 ;
	fcb->puntero_indirecto = 0;

	printf("persisto fcb del archivo %s\n", archivo);


	//TODO ver si esta bien persistirlo asi

	strcat(path,"/");
	strcat(path,archivo);
	FILE * archivo_fcb = fopen(path, "w+");

	fprintf(archivo_fcb,"NOMBRE_ARCHIVO = %s\n",fcb->nombre);
	fprintf(archivo_fcb,"TAMANIO_ARCHIVO = %d\n",fcb->tamanio);
	fprintf(archivo_fcb,"PUNTERO_DIRECTO = %d\n",0);
	fprintf(archivo_fcb,"PUNTERO_INDIRECTO = %d\n",0);

	fclose(archivo_fcb);
	t_config* config = iniciar_config(path);
	fcb->config = config;
	list_add(fcbs,fcb);

	config_destroy(config);
}


int existe_fcb(char*archivo, t_list*fcbs) {

	int i;
	int tamanio = list_size(fcbs);

	for(i=0; i<tamanio; i++) {
		t_fcb*fcb = list_get(fcbs,i);

		if (strcmp(archivo,fcb->nombre) == 0) {
			liberar_fcb(fcb);
			log_debug(logger,"archvivo %s abierto",archivo);
			return 1;
		}
	liberar_fcb(fcb);
	}

	log_debug(logger,"el archvivo %s no existe",archivo);
	return 0;

// ver sino list_any_satisfy() de las commons;

}


void liberar_fcb(t_fcb*fcb) {
	free(fcb->nombre);
	free(fcb);
}

