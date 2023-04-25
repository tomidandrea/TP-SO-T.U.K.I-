#include <planificacion.h>

extern t_config* config;
extern t_list* procesosExecute;
extern t_list* procesosReady;
extern t_list* procesosNew;

void pasarAReady(t_pcb* proceso, t_list* list){
	list_add(list, proceso);
}

void planificar(){

	char* algoritmo = malloc(16);
	algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    char* gradoMultipAux = malloc(16);
    gradoMultipAux = config_get_string_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    int gradoMultip = atoi(gradoMultipAux);

    int i = 0;
    int gradoActual = 0;

	t_pcb* proceso;
    while(list_size(procesosNew) > 0){    //mientras hayan procesos en new

    	if(gradoActual <= gradoMultip){
    		proceso = list_remove(procesosNew, i);
    		list_add(procesosReady, proceso);    //los agrego a ready segun grado multip
    		gradoActual++;
    	}
    	i++;
    }

    // aca ya los tengo en ready
	if(strcmp(algoritmo,"FIFO") == 0){
		planificarFIFO(gradoMultip);
	}
	//aca va HRRN
	//if(strcmp(algoritmo,"HRRN") == 0){
	//	planificarFIFO(procesosNew, procesosReady, procesosExecute, gradoMultip);
	//}

	// esperamos la cpu

	free(algoritmo);
	free(gradoMultipAux);
}




void planificarFIFO(int gradoMultip){

	t_pcb* proceso = malloc(sizeof(list_get(procesosReady, 0)));
	proceso = list_remove(procesosReady, 0);
	list_add(procesosExecute, proceso);

	// mandamo la cosa
	mandar_pcb_a_CPU(proceso);

}

