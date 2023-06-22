#include <utilsFileSystem.h>

extern t_log* logger;


void inicializar_bitarray(t_bitarray*bitarray,size_t cant_bits){
	int i = 0;
	for(i=0;i<cant_bits;i++){
	  bitarray_clean_bit(bitarray,i);
	}
}


void set_archivo_bitmap(char*path,size_t cant_bits) {

	FILE* archivo_bitmap = fopen(path,"w+");
	for(int i = 0;i<cant_bits;i++){
		fputs("0",archivo_bitmap);
	}
	fclose(archivo_bitmap);

}

t_bitarray* mapear_bitmap(size_t cant_bits,size_t cant_bytes, char*path){

	int fd;

	printf("empiezo a mapear bitmap\n");

	//creo bitarray
	void*buffer = malloc(cant_bytes);

	t_bitarray* bitmap = bitarray_create_with_mode(buffer,cant_bytes, LSB_FIRST);

	inicializar_bitarray(bitmap,cant_bits);

	printf("testeo bits del bitarray\n");
	for(int i=0;i<cant_bits;i++)
	printf("%d",bitarray_test_bit(bitmap,i));

	bitmap->bitarray = mapearArchivo(path,&fd);

	close(fd);

	/*
	printf("seteo bits del archivo bitmap mapeado\n");
	for(int i=0;i<cant_bits;i++){
		    	bitmap->bitarray[i]='1';
    }
    */

    printf("archivo bitmap cerrado\n");

    return bitmap;

}

/*
void mapear_bloques(void*bloques,char*path){

	int fd;
	printf("empiezo a mapear bloques\n");
	bloques  = mapearArchivo(path,&fd);

	close(fd);
	printf("archivo bloques cerrado\n");
}

*/

void* mapearArchivo(void*path,int*fd){

     struct stat statbuf;

	 *fd = open(path, O_RDWR);
	    if(*fd < 0){
	        printf("\n\"%p \" no se pudo abrir \n",path);
	        exit(1);
	    }

	    printf("archivo abierto, empezando a mapear\n");

	    printf("path archivo =  %s\n", path);

	    //write(*fd,"hola",4);

	    fstat(*fd,&statbuf);

	    char* buffer = mmap(NULL,statbuf.st_size, PROT_WRITE, MAP_SHARED,*fd,0);
	    if(buffer == MAP_FAILED){
	        printf("Mapping Failed\n");
	        exit(2);
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
