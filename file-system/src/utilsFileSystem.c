#include <utilsFileSystem.h>

extern t_log* logger;
extern size_t cantidad_bloques;
extern t_socket conexionMemoria;

FILE* levantarArchivo(char*path,size_t cant_bytes) {

	FILE*fp;

	fp = fopen(path,"rb+");

	if (fp == NULL) {
		fp = fopen(path,"wb+");
	}
	//printf("archivo %s abierto\n", path);

	ftruncate(fileno(fp),cant_bytes);

	//truncate(path,cant_bytes);

	return fp;
}

char* crear_path_archivo(char*nombre_archivo,char*path_directorio) {
	char* path_archivo = malloc(strlen(path_directorio)+strlen(nombre_archivo)+2);
	strcpy(path_archivo,path_directorio);
	strcat(path_archivo,"/");
	strcat(path_archivo,nombre_archivo);

	return path_archivo;
}


t_bitarray* mapear_bitmap(size_t cant_bytes, FILE*archivo_bitmap){

	void* buffer = mapearArchivo(archivo_bitmap,cant_bytes);

	//creo bitarray

	t_bitarray* bitmap = bitarray_create_with_mode(buffer,cant_bytes, LSB_FIRST);

	if(bitmap!= NULL) {
		log_info(logger,"Archivo bitmap mapeado en bitarray\n");
		//inicializar_bitarray(bitmap);
		mostrar_bitarray(bitmap);
	}


	//bitarray_set_bit(bitmap,5);


	fclose(archivo_bitmap);

    //printf("archivo bitmap cerrado\n");

    return bitmap;

}

void mostrar_bitarray(t_bitarray*bitarray) {

	printf("testeo bits del bitarray\n");

		for(int i=0;i<cantidad_bloques;i++)
		printf("%d",bitarray_test_bit(bitarray,i));
		printf("\n");
}


void* mapearArchivo(FILE*archivo,size_t tamanio){

	    void* buffer = mmap(NULL,tamanio, PROT_WRITE, MAP_SHARED,fileno(archivo),0);
	    if(buffer == MAP_FAILED){
	        printf("Mapping Failed\n");
	        exit(1);
	    }

	    return buffer;

/*
        // desmapeo
	    int err = munmap(buffer, cantidad_bytes);

	    if(err != 0){
	        printf("UnMapping Failed\n");
	        return 0;
	    }
*/

}

void inicializar_bitarray(t_bitarray*bitarray){
	int i = 0;
	for(i=0;i<cantidad_bloques;i++){
	  bitarray_clean_bit(bitarray,i);
	}
}

void setear_n_primeros_bits_en_bitarray(t_bitarray*bitarray,size_t cant_bits, uint32_t indices_bits_asignados[]){

	uint32_t i = 0;
	int j = 0;

	while(se_asignaron_todos_los_bits(indices_bits_asignados,cant_bits) == false) {
			bool valor = bitarray_test_bit(bitarray,i);
			log_info(logger,"Acceso a bitmap - Bloque: %d - Estado: %d",i,valor);
			if(valor == 0){
				indices_bits_asignados[j]= i;
				j++;
				bitarray_set_bit(bitarray,i);
				log_info(logger,"Acceso a bitmap - Bloque: %d - Estado: %d",i,bitarray_test_bit(bitarray,i));
			}
			i++;
		}
}


bool se_asignaron_todos_los_bits(uint32_t indices_bits_asignados[],size_t cant_bits) {
	int i;
	for(i=0;i<cant_bits;i++){
		if(indices_bits_asignados[i]==cantidad_bloques)
			return false;
	}
	return true;
}

void clean_n_bits_bitarray(t_bitarray* bitarray,size_t cant_bits,uint32_t indices_bits_a_limpiar[]) {

	for(int i=0;i<cant_bits;i++) {
		bitarray_clean_bit(bitarray,indices_bits_a_limpiar[i]);
		log_info(logger,"Acceso a bitmap - Bloque: %d - Estado: %d",indices_bits_a_limpiar[i],0);
	}

}


char* recibirNombreArchivo(void* buffer, int* desplazamiento) {
	int tamanio;

	memcpy(&tamanio, buffer + *desplazamiento, sizeof(int));
	*desplazamiento +=sizeof(int);
	char* nombreArchivo = malloc(tamanio);
	memcpy(nombreArchivo, buffer + *desplazamiento, tamanio);
	*desplazamiento+=tamanio;

	return nombreArchivo;
}

void recibirLeerOEscribir(void* buffer, int* desplazamiento, int* puntero, u_int32_t* direc_fisica, int* cant_bytes) {
	memcpy(puntero, buffer + *desplazamiento, sizeof(int));
	*desplazamiento+=sizeof(int);
	memcpy(direc_fisica, buffer + *desplazamiento, sizeof(u_int32_t));
	*desplazamiento+=sizeof(u_int32_t);
	memcpy(cant_bytes, buffer + *desplazamiento, sizeof(int));
	*desplazamiento+=sizeof(int);

}


bool enviar_dato_a_escribir_a_memoria(char*dato_leido, uint32_t direc_fisica) {
	bool result = false;

    t_paquete*paquete=crear_paquete(ESCRIBIR);
    agregar_valor_uint(paquete, &(direc_fisica));
    agregar_a_paquete(paquete,dato_leido,strlen(dato_leido)+1);

    log_info(logger,"Enviando lectura a memoria");
    enviar_paquete(paquete,conexionMemoria);
    eliminar_paquete(paquete);

    int cod_op;
    	if(conexionMemoria!=-1){
    		cod_op = recibir_operacion(conexionMemoria);
    		if(cod_op==MENSAJE) {
               char* mensaje = recibir_mensaje(conexionMemoria,logger);
               if(strcmp(mensaje,"OK")== 0)
                  result = true;
    		}
       } else {
    	   log_error(logger,"No me llego el resultado de memoria");
       }


	return result;
}

char* solicitar_leer_dato_a_memoria(uint32_t direc_fisica,int cant_bytes) {

	    t_paquete *paquete = crear_paquete(LEER);

	    log_info(logger,"Enviando solicitud de lectura a memoria");
		agregar_valor_uint(paquete, &(direc_fisica));
		agregar_valor_estatico(paquete, &(cant_bytes));

		enviar_paquete(paquete,conexionMemoria);
		eliminar_paquete(paquete);
		//hago el recv y devuelvo el valor leido
		char* valor_leido;
		int cod_op;
		if(conexionMemoria!=-1){
			cod_op = recibir_operacion(conexionMemoria);
			if(cod_op==MENSAJE){
			valor_leido = recibir_mensaje(conexionMemoria, logger); //tiene el \0
			}
		} else {
			log_error(logger,"No me llego el resultado de memoria");
		}
		return valor_leido;

}



