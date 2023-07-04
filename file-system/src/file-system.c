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


    //creo un fcb de prueba

     char* path_fcbs = config_get_string_value(config,"PATH_FCB");

    // crear_fcb("PRUEBA",path_fcbs);

    //pruebo truncar, le aumento el tamaño a 32 bytes
    bool truncado =  truncar("PRUEBA",32,path_fcbs,bitmap,archivo_bloques);
    if(truncado)
    	printf("truncado anduvo bien\n");
    else
    	printf("truncado anduvo mal\n");

    fclose(archivo_bloques);
    printf("Archivo de bloques cerrado\n");

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
					 crear_fcb(nombreArchivo,path_fcbs);
		        	 result_operacion = RESULT_OK;
					 break;

				case F_OPEN:
					nombreArchivo = recibirNombreArchivo(buffer, &desplazamiento);
					log_info(logger, "Abrir Archivo: %s ", nombreArchivo);
					result_operacion = existe_fcb(nombreArchivo,path_fcbs);
					 break;

				case F_TRUNCATE:
					int tamanioATruncar;
					nombreArchivo = recibirNombreArchivo(buffer, &desplazamiento);
					memcpy(&tamanioATruncar, buffer + desplazamiento, sizeof(int));
					desplazamiento+=sizeof(int);
					log_info(logger, "Truncar Archivo: %s - Tamaño: %d", nombreArchivo, tamanioATruncar);
					result_operacion = truncar(nombreArchivo,tamanioATruncar,path_fcbs,bitmap,archivo_bloques);
					break;

				case F_READ:
					nombreArchivo = recibirNombreArchivo(buffer, &desplazamiento);
					recibirLeerOEscribir(buffer, &desplazamiento, &puntero, &direc_fisica, &cant_bytes);
					log_info(logger, "Leer Archivo: %s - Puntero: %d - Memoria: %d - Tamaño: %d", nombreArchivo, puntero, direc_fisica, cant_bytes);
				    //result_operacion = leer_archivo(parametros[0],parametros[1],parametros[2]);
				    break;

				case F_WRITE:
					nombreArchivo = recibirNombreArchivo(buffer, &desplazamiento);
					recibirLeerOEscribir(buffer, &desplazamiento, &puntero, &direc_fisica, &cant_bytes);
					log_info(logger, "Escribir Archivo: %s - Puntero: %d - Memoria: %d - Tamaño: %d", nombreArchivo, puntero, direc_fisica, cant_bytes);
					//result_operacion = escribir_archivo(parametros[0],parametros[1],parametros[2]);
					break;


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


void crear_fcb(char* archivo, char*path_directorio) {

	printf("creando fcb del archivo %s\n", archivo);

	char*path_archivo = crear_path_archivo(archivo,path_directorio);

	FILE * archivo_fcb = fopen(path_archivo, "w+");

	fprintf(archivo_fcb,"NOMBRE_ARCHIVO=%s\n",archivo);
	fprintf(archivo_fcb,"TAMANIO_ARCHIVO=%d\n",0);
	fprintf(archivo_fcb,"PUNTERO_DIRECTO=%d\n",0);
	fprintf(archivo_fcb,"PUNTERO_INDIRECTO=%d\n",0);

	fclose(archivo_fcb);
	printf("Archivo fcb  %s  creado\n", archivo);

	free(path_archivo);

}


bool existe_fcb(char*archivo,char*path) {

	strcat(path,"/");
	strcat(path,archivo);

	printf("Verificando si existe archivo: %s\n",archivo);

	printf("path archivo = %s\n",path);

	FILE*fd = fopen(path,"r");

	if(fd == NULL) {
		printf("El archivo %s no existe\n",archivo);
		return false;
	}
	fclose(fd);

	printf("Existe el archivo %s\n",archivo);
	return true;
}



void liberar_fcb(t_fcb*fcb) {
	free(fcb->nombre);
	free(fcb);
}



bool truncar(char* nombre_archivo,int nuevo_tamanio,char*path_directorio,t_bitarray*bitmap, FILE*archivo_bloques) {

	printf("Empiezo a truncar archivo %s a tamanio %d\n",nombre_archivo, nuevo_tamanio);
	bool result = false;
	size_t cant_bloques_nueva;
	cant_bloques_nueva = ceil(nuevo_tamanio/tamanio_bloque);
	printf("cantidad de bloques nuevos %ld\n",cant_bloques_nueva);

	t_fcb*fcb = get_fcb(nombre_archivo,path_directorio);

		if(fcb == NULL){
			liberar_fcb(fcb);
			log_error(logger,"no existe el archivo a truncar, debe ser creado antes\n");
			return result;
		}
	size_t cant_bloques_actual = ceil(fcb->tamanio/tamanio_bloque);     //necesito saber la cantidad de_bloques que tiene antes de truncar
	size_t cant_bloques_indirectos_actual;                          //necesito tambien saber la cantidad de bloques indirectos que tiene antes de truncar para luego saber desde donde hay que escribir/leer en el archivo de bloques
	printf("cantidad de bloques antes de truncar %ld\n",cant_bloques_actual);

	if(cant_bloques_actual != 0)
	    cant_bloques_indirectos_actual = cant_bloques_actual - 1;
	else
		cant_bloques_indirectos_actual = 0;

	printf("cantidad de bloques indirectos antes de truncar %ld\n",cant_bloques_indirectos_actual);


	if(cant_bloques_nueva > cant_bloques_actual) {            //si la cantidad de bloques nueva es mayor a la actual tengo que agregar bloques
		size_t cant_bloques_a_agregar = cant_bloques_nueva - cant_bloques_actual;

		result = agregar_bloques(fcb,cant_bloques_a_agregar,cant_bloques_indirectos_actual,bitmap, archivo_bloques);
	}
	else if (cant_bloques_nueva < cant_bloques_actual) {   // sino si es menor a la actual tengo que liberar bloques
		 size_t cant_bloques_a_liberar = cant_bloques_actual - cant_bloques_nueva;
		 result = liberar_bloques(fcb, cant_bloques_a_liberar, cant_bloques_indirectos_actual,bitmap, archivo_bloques);
	}
	else result = RESULT_OK; // NO HACE NADA: la cantidad de bloques nueva es la misma de antes por lo que no hay que agregar ni eliminar bloques

	fcb->tamanio = nuevo_tamanio;                      // asigno en el fcb el nuevo tamaño.
	printf("Nuevo tamanio: %d\n",fcb->tamanio);

	// persisto fcb en disco

	actualizar_archivo_fcb(fcb,path_directorio);
	liberar_fcb(fcb);

	return result;
}

t_fcb* get_fcb(char*archivo,char*path_directorio) {

char*path_archivo = crear_path_archivo(archivo,path_directorio);
printf("obtengo config del fcb del archivo %s en el path %s\n",archivo,path_archivo);
t_config* config_fcb = iniciar_config(path_archivo);
free(path_archivo);
	if(config_fcb == NULL)
	{
		 config_destroy(config_fcb);
		 return NULL;
	}

	printf("creo fcb en memoria\n");
	t_fcb*fcb = malloc(sizeof(t_fcb));
	fcb->nombre = copiar(config_get_string_value(config_fcb,"NOMBRE_ARCHIVO"));
	fcb->tamanio = config_get_int_value(config_fcb,"TAMANIO_ARCHIVO");
	fcb->puntero_directo = config_get_int_value(config_fcb,"PUNTERO_DIRECTO");
	fcb->puntero_indirecto = config_get_int_value(config_fcb,"PUNTERO_INDIRECTO");
	//fcb->archivo = config_fcb;

	config_destroy(config_fcb);

	printf("NOMBRE ARCHIVO: %s\n",fcb->nombre);
	printf("TAMANIO ARCHIVO: %d\n",fcb->tamanio);
	printf("PUNTERO DIRECTO: %d\n",fcb->puntero_directo);
	printf("PUNTERO INDIRECTO: %d\n",fcb->puntero_indirecto);
	return fcb;

}


void actualizar_archivo_fcb(t_fcb*fcb,char*path_directorio) {
    //el nombre del archivo se supone que no cambia nunca
	char*path_archivo = crear_path_archivo(fcb->nombre,path_directorio);
	t_config*archivo_fcb = iniciar_config(path_archivo);
	free(path_archivo);
	config_set_value(archivo_fcb,"TAMANIO_ARCHIVO",string_itoa(fcb->tamanio));
	config_set_value(archivo_fcb,"PUNTERO_DIRECTO",string_itoa(fcb->puntero_directo));
	config_set_value(archivo_fcb,"PUNTERO_INDIRECTO",string_itoa(fcb->puntero_indirecto));
	config_save(archivo_fcb);
	config_destroy(archivo_fcb);

}

bool agregar_bloques(t_fcb*fcb,size_t cant_bloques_a_agregar, size_t cant_bloques_indirectos_actual, t_bitarray* bitmap,FILE*archivo_bloques) {

	bool result = false;

	if(condicion_para_agregar_bloque_de_punteros(fcb,cant_bloques_a_agregar))           //con esta condicion me doy cuenta si voy a tener que asignar un bloque de punteros ademas de los de datos.
			cant_bloques_a_agregar++;        //le sumo un bloque mas a agregar para el bloque de punteros

	uint32_t bloques_asignados[cant_bloques_a_agregar];
	for(int i = 0;i<cant_bloques_a_agregar;i++)
		bloques_asignados[i] = cantidad_bloques;            //incializo el vector donde voy a guardar los numeros de bloques a asignar con un numero que no pueda ser un nuemero de bloque como puede ser la cantidad de bloques totales


	setear_n_primeros_bits_en_bitarray(bitmap,cant_bloques_a_agregar,bloques_asignados);     //seteo en 1 el primer bit que este libre del bitarray por cada bloque a agregar y en el array voy a guardar que numero de bloque le corresponde a cada uno

	result = asignar_bloques_a_fcb(bloques_asignados,cant_bloques_a_agregar,cant_bloques_indirectos_actual,fcb,bitmap,archivo_bloques);

	return result;
}

bool condicion_para_agregar_bloque_de_punteros(t_fcb*fcb, size_t cant_bloques_a_agregar) {

	if((fcb->tamanio == 0 && cant_bloques_a_agregar >= 2) ||           // si no tengo bloques asignados todavia y tengo que agregar 2 bloques o mas
	   (solo_tiene_un_bloque_asignado(fcb)&& cant_bloques_a_agregar >=1))     // si solo un bloque asignado y tengo que agregar al menos un bloque.

		return true;

  return false;

}

bool solo_tiene_un_bloque_asignado(t_fcb*fcb) {

	if((fcb->tamanio != 0) && (ceil(fcb->tamanio/tamanio_bloque) == 1))
		 return true;

	return false;

}


bool asignar_bloques_a_fcb(uint32_t bloques_asignados[],size_t cant_bloques,size_t cant_bloques_indirectos_actual, t_fcb*fcb,t_bitarray*bitmap,FILE*archivo_bloques) {
     bool result = false;
	 size_t cant_bloques_indirectos;
	 if(fcb->tamanio == 0)  {                  //si no no tiene bloques asignados
	    fcb->puntero_directo = bloques_asignados[0];    //le asigno un bloque al puntero directo
	    printf("Puntero directo asignado = %d\n",fcb->puntero_directo);
	    if (cant_bloques > 1) {                         //si ademas tiene que agregar mas de un bloque
		   cant_bloques_indirectos = cant_bloques - 2;    //calculo la cantidad de bloques que voy a tener que agregar en el bloque de punteros  (los llamo bloques indirectos). Para calcularlo le resto a la cantidad total de bloques a agregar, el bloque para el puntero directo y el bloque para el puntero indirecto
		   result = asignar_bloques_indirectos(fcb,bloques_asignados,cant_bloques,cant_bloques_indirectos,cant_bloques_indirectos_actual,1,archivo_bloques);   // agrego los bloques indirectos
	    }
	}
	else if (solo_tiene_un_bloque_asignado(fcb)){          //si solo tiene un bloque directo asignado
		     cant_bloques_indirectos = cant_bloques - 1;     //ahora la cantidad de bloques indirectos va a ser una unidad mas que en el caso anterior
		     result = asignar_bloques_indirectos(fcb,bloques_asignados,cant_bloques,cant_bloques_indirectos,cant_bloques_indirectos_actual,0,archivo_bloques);
	}
	else {   // ya tiene asignados bloques indirectos (y el directo tambien)

	result = escribir_bloques_en_bloque_de_punteros(fcb->puntero_indirecto, bloques_asignados,cant_bloques,cant_bloques_indirectos_actual,archivo_bloques);
    }

	return result;
}

bool asignar_bloques_indirectos(t_fcb*fcb,uint32_t bloques_asignados[],size_t cant_bloques,size_t cant_bloques_indirectos, size_t cant_bloques_indirectos_actual, int desde, FILE* archivo_bloques) {

	  bool result = false;
	  fcb->puntero_indirecto = bloques_asignados[cant_bloques-1];  // le asigno el ultimo bloque (donde va a estar el bloque de punteros)
	  printf("Puntero indirecto asignado = %d\n",fcb->puntero_indirecto);
	  uint32_t bloques_indirectos[cant_bloques_indirectos];
	  int j=0;
	  int i;
	  for(i=desde;i<cant_bloques-1;i++){
	      bloques_indirectos[j] = bloques_asignados[i];
	      j++;
	  }
	 result = escribir_bloques_en_bloque_de_punteros(fcb->puntero_indirecto,bloques_indirectos,cant_bloques_indirectos,cant_bloques_indirectos_actual,archivo_bloques);

	 return result;
}



bool escribir_bloques_en_bloque_de_punteros(uint32_t puntero_indirecto,uint32_t bloques[],size_t cant_bloques_a_escribir,size_t cant_bloques_actual,FILE* archivo_bloques){

	bool result = false;
	long int offset = puntero_indirecto * tamanio_bloque + cant_bloques_actual * sizeof(uint32_t);
	int result_f_seek, result_f_write;

	result_f_seek = fseek(archivo_bloques,offset,SEEK_SET);   //cambio el puntero a la direccion donde quiero empezar a escribir

	if(result_f_seek == 0) {  // si devuelve 0 significa que fue exitosa

	  result_f_write = fwrite(bloques, sizeof(uint32_t),cant_bloques_a_escribir,archivo_bloques);
	  if (result_f_write == cant_bloques_a_escribir)
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
		   bitarray_clean_bit(bitmap,fcb->puntero_indirecto);
		   log_info(logger,"Acceso a bitmap - Bloque: %d - Estado: %d",fcb->puntero_indirecto,0);
		   fcb->puntero_indirecto = 0;
		   printf("Puntero indirecto = %d\n",fcb->puntero_indirecto);
	   }
	}

	return result;
}


void liberar_bloque_directo (t_fcb*fcb,t_bitarray*bitmap) {
	uint32_t bloque_a_liberar;
	bloque_a_liberar = fcb->puntero_directo;
	bitarray_clean_bit(bitmap,bloque_a_liberar);
	log_info(logger,"Acceso a bitmap - Bloque: %d - Estado: %d",bloque_a_liberar,0);
	fcb->puntero_directo = 0;
	printf("Puntero directo = %d\n",fcb->puntero_directo);
}

bool leer_bloques_a_liberar(uint32_t puntero_indirecto,size_t cant_bloques_a_liberar, uint32_t bloques_a_liberar[],size_t cant_bloques_indirectos_actual, FILE*archivo_bloques) {

	bool result = false;
	int result_f_seek, result_f_read;
	size_t bloque_donde_me_paro =  cant_bloques_indirectos_actual - cant_bloques_a_liberar;

	if(bloque_donde_me_paro >= 0) {
	long int offset = puntero_indirecto * tamanio_bloque + bloque_donde_me_paro*sizeof(uint32_t) ;       //me paro donde voy a empezar a leer

	result_f_seek = fseek(archivo_bloques,offset,SEEK_SET);            // cambio el puntero a donde voy a empezar a leer

	if(result_f_seek == 0) {

	   result_f_read = fread(bloques_a_liberar,sizeof(uint32_t),cant_bloques_a_liberar,archivo_bloques);

	   if (result_f_read == cant_bloques_a_liberar)  // si fread fue exitosa devuelvo true;
		   result = RESULT_OK;
    }

	else result = RESULT_ERROR;

	}
	return result;
}

