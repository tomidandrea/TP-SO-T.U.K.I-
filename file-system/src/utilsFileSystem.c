#include <utilsFileSystem.h>

extern t_log* logger;


FILE* levantarArchivo(char*path,size_t cantidad_bytes) {

	FILE*fp;

	fp = fopen(path,"r+");

	if (fp == NULL) {
		fp = fopen(path,"w+");
	}
	printf("archivo %s abierto\n", path);

	ftruncate(fileno(fp),cantidad_bytes);

	//truncate(path,cantidad_bytes);

	return fp;
}

/*
FILE* set_archivo_bitmap(char*path,size_t cant_bits) {

	FILE* bitmap = fopen(path,"w+");
	for(int i = 0;i<cant_bits;i++){
		fputs("0",bitmap);
	}

	return bitmap;

}
*/

void inicializar_bitarray(t_bitarray*bitarray,size_t cant_bits){
	int i = 0;
	for(i=0;i<cant_bits;i++){
	  bitarray_clean_bit(bitarray,i);
	}
}


t_bitarray* mapear_bitmap(size_t cant_bits,size_t cant_bytes, FILE*archivo_bitmap){


	void* buffer = mapearArchivo(archivo_bitmap,cant_bytes);

	//creo bitarray

	t_bitarray* bitmap = bitarray_create_with_mode(buffer,cant_bytes, LSB_FIRST);

	//inicializar_bitarray(bitmap,cant_bits);

	printf("testeo bits del bitarray\n");

	for(int i=0;i<cant_bits;i++)
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
