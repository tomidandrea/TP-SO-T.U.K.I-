#include <parser.h>


void leerParametros(FILE *archivo, int cantParametros,char** parametros){
	int j;
	char* parametro = malloc(30);
	for(j=0;j<cantParametros;j++){
		fscanf(archivo, "%s",parametro);
		string_trim(&parametro);
		printf("El parametro %s tiene tamanio: %lu\n", parametro, strlen(parametro));
		parametros[j] = copiar(parametro);
	}
	free(parametro);
}


void parsear_instrucciones(char* path, t_list* instrucciones) {

    // Puntero que nos ayuda a leer cada palabra
	char *operacion = malloc(TAMANIO_OPERACION);
    int cant; // Para la cantidad de parametros
    FILE *archivo;
    archivo= fopen(path,"r");

    while(!feof(archivo)){
        //leemos la primer palabra de la instruccion (una operacion)
        fscanf(archivo, "%s",operacion);
        printf("La instruccion %s tiene tamanio: %lu\n", operacion, strlen(operacion));
        //asignamos la cantidad de parametros que tiene la operacion
        cant = cantParametros(operacion);

        //t_instruccion* inst = malloc(sizeof(t_instruccion));
        t_instruccion* inst = malloc(sizeof(t_instruccion*));
        printf("La lista de %d parametros tiene tamanio: %lu\n",cant,  (sizeof(char*) * cant));
        inicializar_instruccion(inst, cant);

        inst->instruccion = copiar(operacion);

        //inst->parametros = string_array_new();

        //leemos los parametros de la instruccion
        leerParametros(archivo,cant,inst->parametros);

        list_add(instrucciones,inst);


    }

    printf("\n------------------\nParser termino bien\n------------------\n\n");
    free(operacion);
    //liberar_instruccion(inst);
    fclose(archivo);

    }

void inicializar_instruccion(t_instruccion* inst, int cantidadParametros){
	inst->instruccion = malloc(TAMANIO_OPERACION);
	inst->parametros = malloc(sizeof(char*) * cantidadParametros);
}


