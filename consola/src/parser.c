#include <consola.h>


void leerParametros(FILE *archivo, int cantParametros,char** parametros){
	int j;
	char* string_aux = string_new();
	for(j=0;j<cantParametros;j++){
		fscanf(archivo, "%s",string_aux);
 		parametros[j] = string_duplicate(string_aux);
	}
	free(string_aux);
}


void parsear_instrucciones(char* path, t_list* instrucciones) {

    // Puntero que nos ayuda a leer cada palabra
	char *operacion = string_new();
    int cant; // Para la cantidad de parametros
    FILE *archivo;
    archivo= fopen(path,"r");

    while(!feof(archivo)){
        //leemos la primer palabra de la instruccion (una operacion)
        fscanf(archivo, "%s",operacion);
        //asignamos la cantidad de parametros que tiene la operacion
        cant = cantParametros(operacion);

        t_instruccion* inst = malloc(sizeof(t_instruccion));
        inst->instruccion = string_duplicate(operacion);
        inst->parametros = string_array_new();

        //leemos los parametros de la instruccion
        leerParametros(archivo,cant,inst->parametros);

        list_add(instrucciones,inst);


    }

    printf("\n------------------\nParser termino bien\n------------------\n\n");
    free(operacion);
    fclose(archivo);

    }
