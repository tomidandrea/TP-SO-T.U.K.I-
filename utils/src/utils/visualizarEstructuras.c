#include <utils/visualizarEstructuras.h>

extern t_log* logger;

void mostrarRegistros(t_registros* registros){
	printf("R AX: %s\n",registros->AX);
	printf("R BX: %s\n",registros->BX);
	printf("R CX: %s\n",registros->CX);
	printf("R DX: %s\n",registros->DX);
	printf("R EAX: %s\n",registros->EAX);
	printf("R EBX: %s\n",registros->EBX);
	printf("R ECX: %s\n",registros->ECX);
	printf("R EDX: %s\n",registros->EDX);
	printf("R RAX: %s\n",registros->RAX);
	printf("R RBX: %s\n",registros->RBX);
	printf("R RCX: %s\n",registros->RCX);
	printf("R RDX: %s\n",registros->RDX);
}

void mostrarListaProcesos(t_list* lista){
	int tamanio = list_size(lista);
	for(int i=0; i<tamanio; i++){
		t_pcb* proceso = list_get(lista, i);
		log_info(logger,"Proceso %d", proceso->pid);
	}
}

char* lista_procesos_string(t_list* lista){
	int tamanio = list_size(lista);
	char* mensaje = copiar("");
		for(int i=0; i<tamanio; i++){
			t_pcb* proceso = list_get(lista, i);
			char* pid = string_itoa(proceso->pid);
			string_append(&mensaje, pid);
			free(pid);
			if(i < tamanio-1){
				string_append(&mensaje, ",");
			}
		}
	return mensaje;
}

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

void mostrarListaSegmentos(tabla_segmentos tabla){
	int cantidad = list_size(tabla);
	for (int i = 0; i < cantidad; ++i) {
		t_segmento* seg = list_get(tabla, i);
		printf("Seg %d: base %d - limite %d\n", seg->id, seg->base, seg->limite);
	}
}

void mostrarTablaHuecos(tabla_segmentos tabla){
	int cantidad = list_size(tabla);
	int tamanio;
	printf("  -- Tabla de huecos - cantidad huecos: %d --  \n", cantidad);
	for (int i = 0; i < cantidad; ++i) {
		t_segmento* seg = list_get(tabla, i);
		tamanio = seg->limite - seg->base;
		printf("Hueco %d: base %d - limite %d - tamaño %d\n", seg->id, seg->base, seg->limite, tamanio);
	}
}


