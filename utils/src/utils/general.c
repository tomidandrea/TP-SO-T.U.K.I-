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

