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
		log_info(logger,"Proceso %d\n", proceso->pid);
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


