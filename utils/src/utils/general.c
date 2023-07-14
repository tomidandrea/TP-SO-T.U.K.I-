#include <utils/general.h>

static t_identificador tablaIdentificadores[] = {
        {"SET",2,SET},{"MOV_OUT",2,MOV_OUT}, {"WAIT",1,WAIT}, {"I/O",1,3}, {"SIGNAL",1,SIGNAL}, {"MOV_IN",2,MOV_IN},
		{"F_OPEN",1,F_OPEN},{"YIELD",0,YIELD},{"F_TRUNCATE",2,F_TRUNCATE},{"F_SEEK",2,F_SEEK},{"CREATE_SEGMENT",2,CREATE_SEGMENT},
		{"F_WRITE",3,F_WRITE},{"F_READ",3,F_READ},{"DELETE_SEGMENT",1,DELETE_SEGMENT},{"F_CLOSE",1,F_CLOSE},{"EXIT",0,EXT}
};

static t_algoritmo tabla_algoritmo_memoria[] = {
		{"FIRST", FIRST_FIT},{"BEST", BEST_FIT},{"WORST", WORST_FIT}
};

t_registros* inicializarRegistros(){
	t_registros* registros = malloc(sizeof(t_registros));
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

int idAlgoritmo(char * algoritmo) {
	for(int i=0;i < 3;i++){
	        t_algoritmo sym;
	        sym = tabla_algoritmo_memoria[i];
	        if(strcmp(sym.operacion,algoritmo)==0) return sym.id;
	    }
	    printf("error algoritmo invalido\n");
	    exit(EXIT_FAILURE);
}

int obtenerIndiceSegmento(tabla_segmentos tabla, int id_segmento) {
	t_segmento* seg;
	int indice = -1;
	for(int k=0; k<list_size(tabla);k++){
		seg = list_get(tabla, k);
		if(seg->id == id_segmento){
			indice = k;
		}
	}
	return indice;
}

t_segmento* obtenerSegmentoPorId(tabla_segmentos tabla_segmentos, int id){
	t_segmento* seg;
	int tamanio = list_size(tabla_segmentos);
	for(int i=0; i<tamanio;i++){
		seg = list_get(tabla_segmentos, i);
		if(seg->id == id)
			return seg;
	}
	return NULL;
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
		char* operacion = list_get(list, i);
		int cant = cantParametros(operacion);
		t_instruccion* inst = inicializar_instruccion(cant);
		inst->instruccion = copiar(operacion);
		//free(operacion);
		for(int j = 0; j<cant; j ++) {
			char* parametro = list_get(list, i + j + 1);
			inst->parametros[j] = copiar(parametro);
			//free(parametro);
		}
		list_add(instrucciones, inst);
		i+= cant + 1;
		//liberar_instruccion(inst);
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

t_instruccion* inicializar_instruccion(int cantidadParametros){
	t_instruccion* inst = malloc(sizeof(t_instruccion));
	inst->parametros = inicializar_parametros(cantidadParametros);
	return inst;
}

void liberar_instrucciones(t_list* instrucciones) {
	int i;
	int cantInstrucciones = list_size(instrucciones);
	for (i = 0; i < cantInstrucciones; ++i) {
		liberar_instruccion(list_get(instrucciones, i));
	}
}

void liberar_instruccion(t_instruccion* inst){
	int cant = cantParametros(inst->instruccion);
	liberar_parametros(inst->parametros, cant);
	free(inst->instruccion);
	free(inst);
}

char** inicializar_parametros(int cantidadParametros){
	char** parametros = calloc(cantidadParametros, sizeof(char*));
	/*for (int i = 0; i < cantidadParametros; ++i) {
		parametros[i] = malloc(sizeof(char*));
	}*/
	return parametros;
}

void liberar_parametros(char** parametros, int cantidadParametros){
	for(int j=0; j<cantidadParametros; ++j)
			free(parametros[j]);
	free(parametros);
}

char* copiar(char* palabra){
	char* tmp = malloc(sizeof(char) * strlen(palabra) + 1);
	memcpy(tmp, palabra, strlen(palabra));
	tmp[strlen(palabra)] = '\0';
	return tmp;
}

int contar(char* cadena, char caracter){
	int comas = 0;
	for (int i = 0; i < strlen(cadena); ++i){
		//printf("caracter %d: %c\n",i,cadena[i]);
	    comas += (cadena[i] == caracter);
	}
	return comas;
}

void copiarRegistros(t_registros* registrosDestino, t_registros* registrosOrigen){
	strncpy(registrosDestino->AX, registrosOrigen->AX, 4);
	strncpy(registrosDestino->BX, registrosOrigen->BX, 4);
	strncpy(registrosDestino->CX, registrosOrigen->CX, 4);
	strncpy(registrosDestino->DX, registrosOrigen->DX, 4);
	strncpy(registrosDestino->EAX, registrosOrigen->EAX, 8);
	strncpy(registrosDestino->EBX, registrosOrigen->EBX, 8);
	strncpy(registrosDestino->ECX, registrosOrigen->ECX, 8);
	strncpy(registrosDestino->EDX, registrosOrigen->EDX, 8);
	strncpy(registrosDestino->RAX, registrosOrigen->RAX, 16);
	strncpy(registrosDestino->RBX, registrosOrigen->RBX, 16);
	strncpy(registrosDestino->RCX, registrosOrigen->RCX, 16);
	strncpy(registrosDestino->RDX, registrosOrigen->RDX, 16);
}

t_pcb* inicializar_pcb(){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	//pcb->instrucciones = list_create();
	pcb->registros = inicializarRegistros();
	pcb->tablaSegmentos = list_create();
	pcb->archivosAbiertos = list_create();
	pcb->tiempoCPU = temporal_create();
	pcb->tiempoEnReady = temporal_create();
	return pcb;
}

void liberar_pcb(t_pcb* pcb){
	liberar_instrucciones(pcb->instrucciones);
	free(pcb->registros);
	temporal_stop(pcb->tiempoEnReady);
	temporal_destroy(pcb->tiempoEnReady);
	temporal_stop(pcb->tiempoCPU);
	temporal_destroy(pcb->tiempoCPU);

	free(pcb);
}

t_contexto* inicializar_contexto(){
	t_contexto* contexto = malloc(sizeof(t_contexto));
	contexto->registros = inicializarRegistros();
	contexto->tablaSegmentos = list_create();
	//contexto->parametros = malloc(sizeof(char*));
	return contexto;
}


void liberar_contexto(t_contexto* contexto){ //por ahora contexto seria lo que nos devuelve cpu
	free(contexto->registros);
	liberar_parametros(contexto->parametros, contexto->cantidadParametros);
	free(contexto);
}

void removerSegmento0(tabla_segmentos tabla_seg){
	t_segmento* seg;
	int tamanio = list_size(tabla_seg);
	for(int i=0; i<tamanio;i++){
		seg = list_get(tabla_seg, i);
		if(seg->id == 0){
			//log_debug(logger, "Remuevo seg0 antes de liberar la tabla de segmentos");
			seg = list_remove(tabla_seg,i);
			break;
		}
	}
}

void liberarTablaSegmentos(void* tablaProceso){
	removerSegmento0((tabla_segmentos)tablaProceso);
	list_destroy_and_destroy_elements((tabla_segmentos)tablaProceso, free);
}

