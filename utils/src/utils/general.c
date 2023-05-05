#include <utils/general.h>

t_registros* registros;

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

t_pcb* recibir_proceso(int socket_cliente) {
	    int size;
		int desplazamiento = 0;
		void * buffer;
		t_list* valores = list_create();
		int tamanio;
        t_pcb* pcb = inicializar_pcb();
		int pid;
		int pc;
		t_list* instrucciones = list_create();

		//printf("hola, recibiendo proceso\n");

		//printf("recibiendo buffer\n");

		buffer = recibir_buffer(&size, socket_cliente);

		//recibir_variable(&pid,buffer,&desplazamiento);      // recibo pid
		//pcb->pid = pid;                                     // actualizo pid en el pcb
		memcpy(&(pcb->pid), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		/*recibir_variable(&pc,buffer,&desplazamiento);       //recivo pc
		pcb->pc = pc;                                      // actualizo pc en el pcb
*/
		memcpy(&(pcb->pc), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		/* alternativa con vectores por tamaño
		recibir_registros(buffer,&desplazamiento, 4,registros_>tamanio_4);
		pcb->registros->tamanio_4 = registros->tamanio_4;
		recibir_registros(buffer,&desplazamiento, 8,registros_>tamanio_8);
		pcb->registros->tamanio_8 = registros->tamanio_8;
		recibir_registros(buffer,&desplazamiento, 16,registros_>tamanio_16);
		pcb->registros->tamanio_16 = registros->tamanio_16;
		*/

		while(desplazamiento < size)                                           //recivo todos los registros y las instrucciones y los meto en una lista de strings
		{
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			char* valor = malloc(tamanio);
			memcpy(valor, buffer+desplazamiento, tamanio); //rompe aca
			desplazamiento+=tamanio;
			list_add(valores, valor);
        }


		t_list* lista_registros = list_take_and_remove(valores,12);     // saco los primeros 12 elementos de la lista de strings que serian los 12 registros y los agrego en una nueva lista. En la anterior lista solo queadn los strings de las instrucciones.
		actualizar_registros(pcb,lista_registros);                          // actualizo los registros de la cpu
		instrucciones = listaAInstrucciones(valores);              // paso de lista de strings a lista de instrucciones
	    pcb->instrucciones = instrucciones;                             // actualizo lista de instrucciones en el pcb

	    return pcb;
}

t_pcb* inicializar_pcb(){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->instrucciones = list_create();
	pcb->registros = inicializarRegistros();
	return pcb;
}


void actualizar_registros(t_pcb *pcb, t_list* lista_registros) {
	/*int cant = list_size(lista_registros);
	for(int i = 0;i<cant;i++) {
		char aux[4];
		char* auxp;
		auxp = list_get(lista_registros,i);
		strncpy(aux, (char *)list_get(lista_registros,i),4);
		log_info(logger, "elemento: %s \n", auxp);
		log_info(logger, "tamanio: %d \n", sizeof(*aux));
		log_info(logger, "tamanio: %d \n", sizeof(aux));
	}*/
	strcpy(registros->AX, (char *) list_get(lista_registros,0));
	strcpy(pcb->registros->AX,registros->AX);
	strcpy(registros->BX , (char *) list_get(lista_registros,1));
	strcpy(pcb->registros->BX, registros->BX);
	strcpy(registros->CX, list_get(lista_registros,2));
	strcpy(pcb->registros->CX, registros->CX);
	strcpy(registros->DX, list_get(lista_registros,3));
	strcpy(pcb->registros->DX, registros->DX);
	strcpy(registros->EAX, list_get(lista_registros,4));
	strcpy(pcb->registros->EAX, registros->EAX);
	strcpy(registros->EBX, list_get(lista_registros,5));
	strcpy(pcb->registros->EBX, registros->EBX);
	strcpy(registros->ECX, list_get(lista_registros,6));
	strcpy(pcb->registros->ECX, registros->ECX);
	strcpy(registros->EDX, list_get(lista_registros,7));
	strcpy(pcb->registros->EDX, registros->EDX);
	strcpy(registros->RAX,list_get(lista_registros,8));
	strcpy(pcb->registros->RAX, registros->RAX);
	strcpy(registros->RBX, list_get(lista_registros,9));
	strcpy(pcb->registros->RBX, registros->RBX);
	strcpy(registros->RCX, list_get(lista_registros,10));
	strcpy(pcb->registros->RCX, registros->RCX);
	strcpy(registros->RDX, list_get(lista_registros,11));
	strcpy(pcb->registros->RDX, registros->RDX);
}

