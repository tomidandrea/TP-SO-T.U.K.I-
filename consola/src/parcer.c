#include <consola.h>



static t_identificador tablaIdentificadores[] = {
        {"SET",2},{"MOV_OUT",2}, {"WAIT",1}, {"I/O",1}, {"SIGNAL",1}, {"MOV_IN",2},
		{"F_OPEN",1},{"YIELD",0},{"F_TRUNCATE",2},{"F_SEEK",2},{"CREATE_SEGMENT",2},
		{"F_WRITE",3},{"F_READ",3},{"DELETE_SEGMENT",1},{"F_CLOSE",1},{"EXIT",0}
};


int cantParametros(char* instruccion){
    for(int i=0;i < CANT_IDENTIFICADORES;i++){
        t_identificador sym;
        sym = tablaIdentificadores[i];
        if(strcmp(sym.operacion,instruccion)==0) return sym.cantParametros;
    }
    printf("error operacion invalida\n"); //si no es un operacion valido devuelve -1
    exit(EXIT_FAILURE);
}

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
