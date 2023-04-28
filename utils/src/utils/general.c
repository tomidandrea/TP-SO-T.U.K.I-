#include <utils/general.h>

static t_identificador tablaIdentificadores[] = {
        {"SET",2},{"MOV_OUT",2}, {"WAIT",1}, {"I/O",1}, {"SIGNAL",1}, {"MOV_IN",2},
		{"F_OPEN",1},{"YIELD",0},{"F_TRUNCATE",2},{"F_SEEK",2},{"CREATE_SEGMENT",2},
		{"F_WRITE",3},{"F_READ",3},{"DELETE_SEGMENT",1},{"F_CLOSE",1},{"EXIT",0}
};

void logearInstrucciones(t_list* instrucciones, t_log* logger){
	int cant = list_size(instrucciones);
	        for(int i = 0;i<cant;i++) {
	    	t_instruccion* inst = list_get(instrucciones,i);

	        log_info(logger, "Instruccion: %s", inst->instruccion);

	        int cant_parametros = cantParametros(inst->instruccion);

	    	for(int i=0; i<cant_parametros; i++) {
	    		log_info(logger, "Parametro %d: %s", i, inst->parametros[i]);
	    	}
	        printf("------------\n");
	    }
}

int cantParametros(char* instruccion){
    for(int i=0;i < CANT_IDENTIFICADORES;i++){
        t_identificador sym;
        sym = tablaIdentificadores[i];
        if(strcmp(sym.operacion,instruccion)==0) return sym.cantParametros;
    }
    printf("error operacion invalida\n"); //si no es un operacion valido devuelve -1
    exit(EXIT_FAILURE);
}

t_log* iniciar_logger(char* file, char *process_name, bool is_active_console, t_log_level level)
{
	t_log* nuevo_logger = log_create(file, process_name,is_active_console, level);

	return nuevo_logger;
}

t_config* iniciar_config (char* path){
	t_config* nuevo_config = config_create(path);
	if(nuevo_config== NULL){
		printf("No se pudo crear el config\n");
	}

	return nuevo_config;
}

t_list* listaAInstrucciones(t_list* list) {
	t_list* instrucciones = list_create();
	int tamanio = list_size(list);
	int i = 0;

	while (i < tamanio) {
		t_instruccion* inst = malloc(sizeof(t_instruccion));
		char* operacion = list_get(list, i);
		int cant = cantParametros(operacion);
		inst->instruccion = string_duplicate(operacion);
		inst->parametros = string_array_new();
		for(int j = 0; j<cant; j ++) {
			char* parametro = list_get(list, i + j + 1);
			inst->parametros[j] = string_duplicate(parametro);
		}
		list_add(instrucciones, inst);
		i+= cant + 1;
	}

	return instrucciones;
}

