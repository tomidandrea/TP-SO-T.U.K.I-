#include <parser.h>

void leerParametros(FILE *archivo, int cantParametros,char** parametros){
	int j;
	char* parametro = malloc(30);
	for(j=0;j<cantParametros;j++){
		fscanf(archivo, "%s",parametro);
		//printf("El parametro %s tiene tamanio: %lu\n", parametro, strlen(parametro));
		parametros[j] = copiar(parametro);
	}
	free(parametro);
}

void parsear_instrucciones(char* path, t_list* instrucciones) {


    int cant; // Para la cantidad de parametros
    FILE *archivo;
    archivo= fopen(path,"r");

    while(!feof(archivo)){
    	// Puntero que nos ayuda a leer cada palabra
		char *operacion = malloc(TAMANIO_OPERACION);
        //leemos la primer palabra de la instruccion (una operacion)
        fscanf(archivo, "%s ",operacion);
        //asignamos la cantidad de parametros que tiene la operacion
        cant = cantParametros(operacion);

        t_instruccion* inst = inicializar_instruccion(cant);
        inst->instruccion = copiar(operacion);

        //leemos los parametros de la instruccion
        leerParametros(archivo,cant,inst->parametros);
        list_add(instrucciones,inst);
        free(operacion);
    }

    printf("\n------------------\nParser termino bien\n------------------\n\n");
    fclose(archivo);
}


