#include <utils/general.h>

static t_identificador tablaIdentificadores[] = {
        {"SET",2,SET},{"MOV_OUT",2,MOV_OUT}, {"WAIT",1,WAIT}, {"I/O",1,3}, {"SIGNAL",1,SIGNAL}, {"MOV_IN",2,MOV_IN},
		{"F_OPEN",1,F_OPEN},{"YIELD",0,YIELD},{"F_TRUNCATE",2,F_TRUNCATE},{"F_SEEK",2,F_SEEK},{"CREATE_SEGMENT",2,CREATE_SEGMENT},
		{"F_WRITE",3,F_WRITE},{"F_READ",3,F_READ},{"DELETE_SEGMENT",1,DELETE_SEGMENT},{"F_CLOSE",1,F_CLOSE},{"EXIT",0,EXT}
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

t_registros* inicializarRegistros(){
	//registros = malloc(4*4+4*8+4*16);
	t_registros* registros = malloc(sizeof(t_registros));
	/*registros->AX = malloc(4);
	registros->BX = malloc(4);
	registros->CX = malloc(4);
	registros->DX = malloc(4);
	registros->EAX = malloc(8);
	registros->EBX = malloc(8);
	registros->ECX = malloc(8);
	registros->EDX = malloc(8);
	registros->RAX = malloc(16);
	registros->RBX = malloc(16);
	registros->RCX = malloc(16);
	registros->RDX = malloc(16);*/

	return registros;
}

int id(char * instruccion) {
	for(int i=0;i < CANT_IDENTIFICADORES;i++){
	        t_identificador sym;
	        sym = tablaIdentificadores[i];
	        if(strcmp(sym.operacion,instruccion)==0) return sym.id;
	    }
	    printf("error operacion invalida\n");
	    exit(EXIT_FAILURE);
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

void liberar_instruccion(t_instruccion* inst) {
	int i;
	int cant = cantParametros(inst->instruccion);
	for(i=0; i<cant; ++i)
		free(inst->parametros[i]);
	free(inst->parametros);
	free(inst->instruccion);
	//string_array_destroy(instruccion->parametros);
	free(inst);
}

char* copiar(char* palabra){
	char* tmp = malloc(sizeof(char) * strlen(palabra) + 1);
	memcpy(tmp, palabra, strlen(palabra));
	tmp[strlen(palabra)] = '\0';
	return tmp;
}
