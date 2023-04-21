#include <utils.h>

int processID=0;

t_pcb* crearPCB(t_list* listaInstrucciones){

	t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->pid = ++processID;
    pcb->pc = 0;
    pcb->instrucciones = list_duplicate(listaInstrucciones);
    pcb->estado = NEW;

    return pcb;
}
