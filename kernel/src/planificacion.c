#include <planificacion.h>



void pasarAReady(t_pcb* proceso){
		proceso -> estado = READY;
}

void planificarFIFO(t_list* procesos, int gradoMultip){
	//[pcb1,pcb2,pcb3,pcb4]
	int cant = list_size(procesos);

	for(int i = 0;i<gradoMultip;i++) {
			t_pcb* pcbActual = list_get(procesos,i);
			pasarAReady(pcbActual);
			printf("\n\nEstado:%d\n\n", pcbActual -> estado);
	}


}
