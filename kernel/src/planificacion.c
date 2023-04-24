#include <planificacion.h>

void pasarAReady(t_pcb* proceso, t_list* list){
	list_add(list, proceso);
}

void planificar(t_config* config, t_list* procesosExecute,t_list* procesosReady,t_list* procesosNew){

	char* algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    char* gradoMultipAux = config_get_string_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    int gradoMultip = atoi(gradoMultipAux);
    /*
    mientras hayan procesos en new
    los agrego a ready segun grado multip
    */

    // aca ya los tengo en ready
	if(strcmp(algoritmo,"FIFO") == 0){
		planificarFIFO(procesosNew, procesosReady, procesosExecute, gradoMultip);
	}

	// esperamos la cpu

}




void planificarFIFO(t_list* procesosReady, t_list* procesosExecute, t_list* gradoMultip){

	t_pcb* proceso = list_remove(procesosReady, 0);
	list_add(procesosExecute, proceso);

	// mandamo la cosa


}

