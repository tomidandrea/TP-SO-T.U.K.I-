#include <utilsFileSystem.h>

extern t_log* logger;
extern size_t cantidad_bloques;

FILE* levantarArchivo(char*path,size_t cant_bytes) {

	FILE*fp;

	fp = fopen(path,"r+");

	if (fp == NULL) {
		fp = fopen(path,"w+");
	}
	printf("archivo %s abierto\n", path);

	ftruncate(fileno(fp),cant_bytes);

	//truncate(path,cantidad_bytes);

	return fp;
}



t_bitarray* mapear_bitmap(size_t cant_bytes, FILE*archivo_bitmap){

	void* buffer = mapearArchivo(archivo_bitmap,cant_bytes);

	//creo bitarray

	t_bitarray* bitmap = bitarray_create_with_mode(buffer,cant_bytes, LSB_FIRST);

	//inicializar_bitarray(bitmap);

	//bitarray_set_bit(bitmap,5);

	printf("testeo bits del bitarray\n");

	for(int i=0;i<cantidad_bloques;i++)
	printf("%d",bitarray_test_bit(bitmap,i));
	printf("\n");

	fclose(archivo_bitmap);

    printf("archivo bitmap cerrado\n");

    return bitmap;

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

void setear_n_primeros_bits_en_bitarray(t_bitarray*bitmap,size_t cant_bits, uint32_t indices_bits_asignados[]){

	uint32_t i = 0;
	int j = 0;

	while(se_asignaron_todos_los_bits(indices_bits_asignados,cant_bits) == false) {
			size_t valor = bitarray_test_bit(bitmap,i);
			if(valor == 0){
				indices_bits_asignados[j]= i+1;
				j++;
				bitarray_set_bit(bitmap,i);
			}
			i++;
		}
}


bool se_asignaron_todos_los_bits(uint32_t indices_bits_asignados[],size_t cant_bits) {
	int i;
	for(i=0;i<cant_bits;i++){
		if(indices_bits_asignados[i]==0)
			return false;
	}
	return true;
}


void recibo_parametros(t_socket socket_cliente,char** parametros) {

	int size,tamanio;
	int desplazamiento = 0;
	int i=0;
	void * buffer;

	buffer = recibir_buffer(&size, socket_cliente);

	while(desplazamiento < size)
		{
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			char* valor = malloc(tamanio);
			memcpy(valor, buffer+desplazamiento, tamanio);
			desplazamiento+=tamanio;
			strcpy(parametros[i],valor);
			i++;
        }
}



