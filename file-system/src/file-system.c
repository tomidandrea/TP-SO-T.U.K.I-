#include <file-system.h>

t_log* logger;
t_config* config;
t_socket conexionMemoria;

bool RESULT_OK = true;
bool RESULT_ERROR = false;

size_t cantidad_bloques = 0;
int tamanio_bloque = 0;

int main(int argc, char* argv[]) {

	size_t cantidad_bytes = 0;
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
	FILE* archivo_bitmap = levantarArchivo(path_bitmap,cantidad_bytes);
	free(path_bitmap);

	//mapeo el archivo bitmap en un bitarray
	t_bitarray* bitmap = mapear_bitmap(cantidad_bytes,archivo_bitmap);  // devuelve bitarray creado despues de mapear

     //levanto archivo de bloques (si no existe lo creo)

	 char* path_bloques = config_get_string_value(config,"PATH_BLOQUES");
     tamanio_bloque = config_get_int_value(superbloque,"BLOCK_SIZE");
     int tamanio_total = cantidad_bloques*tamanio_bloque;

     FILE*archivo_bloques = levantarArchivo(path_bloques,tamanio_total);
     free(path_bloques);
     //TODO ver si es mejor directamente usar fseek,fread y frwite (no mapearlo en memoria)

    //creo un fcb de prueba

     char* path_fcbs = config_get_string_value(config,"PATH_FCB");

     //crear_fcb("/pruebafcb",fcbs,path_fcbs);

     /* TODO recorrer cada archivo fcb del directorio de fcbs y mapearlos en una lista  para manejarlos en memoria

     fcbs = recorrerDirectorioFCB(path_fcbs);
     */

    //inicio servidor para kernel
	t_socket server_fd = iniciarServidor(config, logger,"PUERTO_ESCUCHA");

	t_socket socket_cliente = esperar_cliente(server_fd, logger);


	while(1) {
		if(socket_cliente != -1) {
		int cod_op = recibir_operacion(socket_cliente);

		int size,cant_bytes, puntero;
		u_int32_t direc_fisica;
		int desplazamiento = 0;
		void * buffer;
		char* nombreArchivo;
		buffer = recibir_buffer(&size, socket_cliente);

		switch (cod_op) {

		        case CREAR_ARCHIVO:
		        	 nombreArchivo = recibirNombreArchivo(buffer, &desplazamiento);
					 log_info(logger, "Crear Archivo: %s ", nombreArchivo);
					 crear_fcb(nombreArchivo,fcbs,path_fcbs);
		        	 result_operacion = RESULT_OK;
					 break;

				case F_OPEN:
					nombreArchivo = recibirNombreArchivo(buffer, &desplazamiento);
					log_info(logger, "Abrir Archivo: %s ", nombreArchivo);
					result_operacion = existe_fcb(nombreArchivo,fcbs);
					 break;

				case F_TRUNCATE:
					int tamanioATruncar;
					nombreArchivo = recibirNombreArchivo(buffer, &desplazamiento);
					memcpy(&tamanioATruncar, buffer + desplazamiento, sizeof(int));
					desplazamiento+=sizeof(int);
					log_info(logger, "Truncar Archivo: %s - Tamaño: %d", nombreArchivo, tamanioATruncar);
					result_operacion = truncar(nombreArchivo,tamanioATruncar, fcbs,bitmap,archivo_bloques);
					break;

				case F_READ:
					nombreArchivo = recibirNombreArchivo(buffer, &desplazamiento);
					recibirLeerOEscribir(buffer, &desplazamiento, &puntero, &direc_fisica, &cant_bytes);

				    //result_operacion = leer_archivo(parametros[0],parametros[1],parametros[2]);
				    break;

				case F_WRITE:
					nombreArchivo = recibirNombreArchivo(buffer, &desplazamiento);
					recibirLeerOEscribir(buffer, &desplazamiento, &puntero, &direc_fisica, &cant_bytes);
					//result_operacion = escribir_archivo(parametros[0],parametros[1],parametros[2]);
					break;
				default:
					log_error(logger,"Se cerró la conexión");
					exit(1);


		free(nombreArchivo);
		}

		if(result_operacion == RESULT_OK) {
			enviar_mensaje("OPERACION_OK",socket_cliente);
		}
		else {
			enviar_mensaje("OPERACION_ERROR",socket_cliente);
		}
	 }
		else  {
		log_error(logger, "Se rompio la conexion con el cliente");
		exit(1);
	 }

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



bool truncar(char* nombre_archivo,int nuevo_tamanio,t_list*fcbs,t_bitarray*bitmap, FILE*archivo_bloques) {

	bool result = false;
	size_t cant_bloques_nueva;
	cant_bloques_nueva = ceil(nuevo_tamanio/tamanio_bloque);

	t_fcb*fcb = get_fcb(nombre_archivo,fcbs);
		if(fcb == NULL){
			log_debug(logger,"no existe el archivo, debe ser creado antes\n");
			return result;
		}

	size_t cant_bloques_actual = ceil(fcb->tamanio/tamanio_bloque);
	fcb->tamanio = nuevo_tamanio;

	if(cant_bloques_nueva > cant_bloques_actual) {
		size_t cant_bloques_a_agregar = cant_bloques_nueva - cant_bloques_actual;
		result = agregar_bloques(fcb,cant_bloques_a_agregar,bitmap, archivo_bloques);
	}
	else if (cant_bloques_nueva < cant_bloques_actual) {
		 size_t cant_bloques_a_liberar = cant_bloques_actual - cant_bloques_nueva;
		 size_t cant_bloques_indirectos_actual = cant_bloques_actual - 1;    //necesito tambien saber la cantidad de bloques indirectos que tenia antes para luego saber desde donde hay que leerlos en el archivo de bloques para eliminarlos
		 if(cant_bloques_indirectos_actual >= 0)
		 result = liberar_bloques(fcb, cant_bloques_a_liberar, cant_bloques_indirectos_actual,bitmap, archivo_bloques);
	}
	else result = RESULT_OK; //NO HACE NADA: la cantidad de bloques nueva es la misma de antes por lo que no hay que agregar ni eliminar bloques

	// persisto fcb en disco

	actualizar_archivo_fcb(fcb);
	liberar_fcb(fcb);

	return result;
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


void actualizar_archivo_fcb(t_fcb*fcb) {
    //el nombre del archivo se supone que no cambia nunca
	config_set_value(fcb->config,"TAMANIO_ARCHIVO",string_itoa(fcb->tamanio));
	config_set_value(fcb->config,"PUNTERO_DIRECTO",string_itoa(fcb->puntero_directo));
	config_set_value(fcb->config,"PUNTERO_INDIRECTO",string_itoa(fcb->puntero_indirecto));

}

bool agregar_bloques(t_fcb*fcb,size_t cant_bloques_a_agregar,t_bitarray* bitmap,FILE*archivo_bloques) {

	bool result = false;
	uint32_t bloques_asignados[cant_bloques_a_agregar];

	if(fcb->puntero_indirecto == 0 && cant_bloques_a_agregar > 1)             //si no tiene bloque indirecto asignado
		cant_bloques_a_agregar++;        //le sumo un bloque mas a agregar para el bloque de punteros

	setear_n_primeros_bits_en_bitarray(bitmap,cant_bloques_a_agregar,bloques_asignados);

	result = asignar_bloques_a_fcb(bloques_asignados,cant_bloques_a_agregar,fcb,bitmap,archivo_bloques);

	return result;
}




bool asignar_bloques_a_fcb(uint32_t bloques_asignados[],size_t cant_bloques,t_fcb*fcb,t_bitarray*bitmap,FILE*archivo_bloques) {
     bool result = false;
	 size_t cant_bloques_indirectos;
	 if(fcb->puntero_directo == 0)  { //esto significa que no tiene bloques asignados
	    fcb->puntero_directo = bloques_asignados[0];
	    if (cant_bloques > 1) {
		   cant_bloques_indirectos = cant_bloques - 2;
		   result = asignar_bloques_indirectos(fcb,bloques_asignados,cant_bloques,cant_bloques_indirectos,1,archivo_bloques);
	    }
	}
	else if (fcb->puntero_indirecto == 0){          //si solo tiene un bloque directo asignados pero no tiene todavia los indirectos
		     cant_bloques_indirectos = cant_bloques - 1;
		     result = asignar_bloques_indirectos(fcb,bloques_asignados,cant_bloques,cant_bloques_indirectos,0,archivo_bloques);
	}
	else {   // ya tiene asignados bloques indirectos (y el directo tambien)

	result = escribir_bloques_en_bloque_de_punteros(fcb->puntero_indirecto, bloques_asignados,cant_bloques,archivo_bloques);
    }

	return result;
}

bool asignar_bloques_indirectos(t_fcb*fcb,uint32_t bloques_asignados[],size_t cant_bloques,size_t cant_bloques_indirectos, int desde, FILE* archivo_bloques) {

	  bool result = false;
	  fcb->puntero_indirecto = bloques_asignados[cant_bloques-1];  // le asigno el ultimo bloque (donde va a estar el bloque de punteros)
	  uint32_t bloques_indirectos[cant_bloques_indirectos];
	  int j=0;
	  int i;
	  for(i=desde;i<cant_bloques-1;i++){
	      bloques_indirectos[j] = bloques_asignados[i];
	      j++;
	  }
	 result = escribir_bloques_en_bloque_de_punteros(fcb->puntero_indirecto,bloques_indirectos,cant_bloques_indirectos,archivo_bloques);

	 return result;
}



bool escribir_bloques_en_bloque_de_punteros(uint32_t puntero_indirecto,uint32_t bloques[],size_t cant_bloques,FILE* archivo_bloques){

	bool result = false;
	long int offset = (puntero_indirecto - 1) * tamanio_bloque;
	int result_f_seek, result_f_write;

	result_f_seek = fseek(archivo_bloques,offset,SEEK_SET);   //cambio el puntero a la direccion donde quiero empezar a escribir

	if(result_f_seek == 0) {  // si devuelve 0 significa que fue exitosa

	  for(int i=0;i<cant_bloques;i++) {   //le resto 1 a cada bloque antes de escribirlo porque los bloques van desde el 0 hasta la cantidad de bloques total - 1
		  bloques[i]--;
	  }
	  result_f_write = fwrite(bloques, sizeof(uint32_t),cant_bloques,archivo_bloques);
	  if (result_f_write == 0)
		  result = RESULT_OK;
	  else result = RESULT_ERROR;
	}

	else result = RESULT_ERROR;

	return result;
}


bool liberar_bloques(t_fcb*fcb,size_t cant_bloques_a_liberar, size_t cant_bloques_indirectos_actual, t_bitarray* bitmap, FILE*archivo_bloques) {

	bool result = false;

	if(cant_bloques_a_liberar > cant_bloques_indirectos_actual) {     // significa que debo eliminar todos los bloques (el bloque directo, los bloques que estan en el indirecto y el bloque de punteros indiectos)

    	   liberar_bloque_directo(fcb,bitmap);    // libero el bloque directo

    	   if(cant_bloques_indirectos_actual > 0)   // significa que tambien tengo bloques indirectos a liberar
    		   cant_bloques_a_liberar = cant_bloques_indirectos_actual;

      result = RESULT_OK;
	}
	else if (cant_bloques_a_liberar == cant_bloques_indirectos_actual) {  //significa que no voy a liberar el bloque directo pero el resto si
		cant_bloques_a_liberar = cant_bloques_indirectos_actual;
	}
	//libero los bloques que estan en el bloque de punteros indirectos

	if(fcb->puntero_indirecto !=0) {                      //libero n bloques que estan en el bloque de punteros indirectos (solo si es que hay bloques indirectos)
	   uint32_t bloques_a_liberar[cant_bloques_a_liberar];
	   result = leer_bloques_a_liberar(fcb->puntero_indirecto,cant_bloques_a_liberar,bloques_a_liberar,cant_bloques_indirectos_actual,archivo_bloques);
	   clean_n_bits_bitarray(bitmap,cant_bloques_a_liberar,bloques_a_liberar);
	   if(cant_bloques_a_liberar == cant_bloques_indirectos_actual) { //libero el bloque de punteros indirectos
		  fcb->puntero_indirecto = 0;
	      bitarray_clean_bit(bitmap,fcb->puntero_indirecto - 1);
	   }
	}

	return result;
}


void liberar_bloque_directo (t_fcb*fcb,t_bitarray*bitmap) {
	uint32_t bloque_a_liberar;
	bloque_a_liberar = fcb->puntero_directo;
	fcb->puntero_directo = 0;
	bitarray_clean_bit(bitmap,bloque_a_liberar-1);
}

bool leer_bloques_a_liberar(uint32_t puntero_indirecto,size_t cant_bloques_a_liberar, uint32_t bloques_a_liberar[],size_t cant_bloques_indirectos_actual, FILE*archivo_bloques) {

	bool result = false;
	int result_f_seek, result_f_read;
	size_t bloque_donde_me_paro =  cant_bloques_indirectos_actual - cant_bloques_a_liberar;

	if(bloque_donde_me_paro >= 0) {
	long int offset = (puntero_indirecto - 1) * tamanio_bloque + bloque_donde_me_paro*sizeof(uint32_t) ;       //me paro donde voy a empezar a leer

	result_f_seek = fseek(archivo_bloques,offset,SEEK_SET);            // cambio el puntero a donde voy a empezar a leer

	if(result_f_seek == 0) {

	   result_f_read = fread(bloques_a_liberar,sizeof(uint32_t),cant_bloques_a_liberar,archivo_bloques);

	   if (result_f_read == 0)  // si fread fue exitosa devuelvo true;
		   result = RESULT_OK;
	   else result = RESULT_ERROR;
    }

	else result = RESULT_ERROR;

	}
	return result;
}

