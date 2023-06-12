#include <utilsFileSystem.h>

int mapearArchivo(void*buffer,void*path,size_t cantidad_bytes){

	 int fd = open(path, O_RDWR);
	    if(fd < 0){
	        printf("\n\"%p \" no se pudo abrir \n",path);
	        exit(1);
	    }

	    printf("archivo abierto, empezando a mapear\n");

	    buffer = mmap(NULL,cantidad_bytes,
	            PROT_READ|PROT_WRITE,MAP_SHARED,
	            fd,0);
	    if(buffer == MAP_FAILED){
	        printf("Mapping Failed\n");
	        return 0;
	    }
	    close(fd);
        printf("archivo cerrado\n");


        // ver como escribir lo del buffer al archivo mapeado

        /*
        ssize_t n = write(1,buffer,cantidad_bytes);
       	    if(n != cantidad_bytes){
       	        printf("Write failed\n");
       	    }

        */

        // desmapeo
	    int err = munmap(buffer, cantidad_bytes);

	    if(err != 0){
	        printf("UnMapping Failed\n");
	        return 0;
	    }

	    return 1;

}

