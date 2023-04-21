#include <planificacion.h>

void planificarFIFO(t_pcb* pcb, t_list* procesos){

	list_add(procesos, pcb);
	printf("\nAgregue el proceso:%d\n",pcb->pid);
}
