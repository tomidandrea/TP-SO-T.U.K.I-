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

    //creo un fcb de prueba

     char* path_fcbs = config_get_string_value(config,"PATH_FCB");

     crear_fcb("/pruebafcb",fcbs,path_fcbs);

     /* TODO recorrer cada archivo fcb del directorio de fcbs y mapearlos en una lista  para manejarlos en memoria

     fcbs = recorrerDirectorioFCB(path_fcbs);
     */

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
					log_info(logger, "Truncar Archivo: %s - Tamaño: %s", parametros[0], parametros[1]);
					int nuevo_tamanio = atoi(parametros[1]);
					result_operacion = truncar_archivo(parametros[0],nuevo_tamanio,tamanio_bloque,fcbs,bitmap,archivo_bloques);
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



bool truncar_archivo(char* nombre_archivo,int nuevo_tamanio, size_t tamanio_bloque,t_list*fcbs,t_bitarray*bitmap, FILE*archivo_bloques) {

	size_t cant_bloques_nueva;
	cant_bloques_nueva = ceil(nuevo_tamanio/tamanio_bloque);

	t_fcb*fcb = get_fcb(nombre_archivo,fcbs);
		if(fcb == NULL){
			log_debug(logger,"no existe el archivo, debe ser creado antes\n");
			return false;
		}

	size_t cant_bloques_actual = ceil(fcb->tamanio/tamanio_bloque);
	fcb->tamanio = nuevo_tamanio;

	if(cant_bloques_nueva > cant_bloques_actual) {
		size_t cant_bloques_a_agregar = cant_bloques_nueva - cant_bloques_actual;
		agregar_bloques(fcb,cant_bloques_a_agregar,bitmap, archivo_bloques);
	}
	else if (cant_bloques_nueva < cant_bloques_actual) {
		 size_t cant_bloques_a_liberar = cant_bloques_actual - cant_bloques_nueva;
		 size_t cant_bloques_indirectos_actual = cant_bloques_actual - 1;    //necesito tambien saber la cantidad de bloques indirectos que tenia antes para luego saber desde donde hay que leerlos en el archivo de bloques para eliminarlos
		 if(cant_bloques_indirectos_actual >= 0)
		 liberar_bloques(fcb, cant_bloques_a_liberar, cant_bloques_indirectos_actual,bitmap, archivo_bloques);
	}
	else ; //NO HACE NADA: la cantidad de bloques nueva es la misma de antes por lo que no hay que agregar ni eliminar bloques

	return true;
}



t_fcb* get_fcb(char*archivo, t_list*fcbs) {

	int i;
	int tamanio = list_size(fcbs);
	for(i=0; i<tamanio; i++) {
		t_fcb*fcb = list_get(fcbs,i);
		if (strcmp(archivo,fcb->nombre) == 0)
			return fcb;
	 liberar_fcb(fcb);
	}
	return NULL;
}

void agregar_bloques(t_fcb*fcb,size_t cant_bloques,t_bitarray* bitmap,FILE*archivo_bloques) {

	uint32_t bloques_asignados[cant_bloques];

	if(fcb->puntero_indirecto == 0 && cant_bloques > 1)             //si no tiene bloque indirecto asignado
		cant_bloques++;        //le sumo un bloque mas a agregar para el bloque de punteros

	setear_n_primeros_bits_en_bitarray(bitmap,cant_bloques,bloques_asignados);

	asignar_bloques_a_fcb(bloques_asignados,cant_bloques,fcb,bitmap,archivo_bloques);
}




void asignar_bloques_a_fcb(uint32_t bloques_asignados[],size_t cant_bloques,t_fcb*fcb,t_bitarray*bitmap,FILE*archivo_bloques) {

	 size_t cant_bloques_indirectos;
	if(fcb->puntero_directo == 0)  { //esto significa que no tiene bloques asignados
	 fcb->puntero_directo = bloques_asignados[0];
	  if (cant_bloques > 1) {
		  cant_bloques_indirectos = cant_bloques - 2;
		  asignar_bloques_indirectos(fcb,bloques_asignados,cant_bloques,cant_bloques_indirectos,1,archivo_bloques);
	  }
	}
	else if (fcb->puntero_indirecto == 0){          //si solo tiene un bloque directo asignados pero no tiene todavia los indirectos
		cant_bloques_indirectos = cant_bloques - 1;
		asignar_bloques_indirectos(fcb,bloques_asignados,cant_bloques,cant_bloques_indirectos,0,archivo_bloques);
	}
	else {   // ya tiene asignados bloques indirectos (y el directo tambien)

	escribir_bloques_en_bloque_de_punteros(fcb->puntero_indirecto, bloques_asignados,cant_bloques,archivo_bloques);
}
}

void asignar_bloques_indirectos(t_fcb*fcb,uint32_t bloques_asignados[],size_t cant_bloques,size_t cant_bloques_indirectos, int desde, FILE* archivo_bloques) {

	  fcb->puntero_indirecto = bloques_asignados[cant_bloques-1];  // le asigno el ultimo bloque (donde va a estar el bloque de punteros)
	  uint32_t bloques_indirectos[cant_bloques_indirectos];
	  int j=0;
	  int i;
	  for(i=desde;i<cant_bloques-1;i++){
	      bloques_indirectos[j] = bloques_asignados[i];
	      j++;
	  }
	 escribir_bloques_en_bloque_de_punteros(fcb->puntero_indirecto,bloques_indirectos,cant_bloques_indirectos,archivo_bloques);
}



void escribir_bloques_en_bloque_de_punteros(uint32_t puntero_indirecto,uint32_t bloques[],size_t cant_bloques,FILE* archivo_bloques){

}


void liberar_bloques(t_fcb*fcb,size_t cant_bloques_a_liberar, size_t cant_bloques_indirectos_actual, t_bitarray* bitmap, FILE*archivo_bloques) {


	if(fcb->puntero_indirecto == 0){     //solo tiene un bloque (apuntado por el puntero directo)
	 uint32_t bloque_a_liberar;
	 bloque_a_liberar = fcb->puntero_directo;
	 fcb->puntero_directo = 0;
	 bitarray_clean_bit(bitmap,bloque_a_liberar-1);
	}
	else  {    //sino significa que se debe empezar a eliminar desde los bloques que estan apuntados de forma indirecta
    uint32_t bloques_a_liberar[cant_bloques_a_liberar];
    leer_bloques_a_liberar(fcb->puntero_indirecto,cant_bloques_a_liberar,bloques_a_liberar,cant_bloques_indirectos_actual,archivo_bloques);

	}

}

void leer_bloques_a_liberar(uint32_t puntero_indirecto,size_t cant_bloques_,uint32_t bloques_a_liberar[],size_t cant_bloques_indirectos_actual, FILE*archivo_bloques) {


}
