#ifndef PLANIF_H
#define PLANIF_H

#include <utils.h>


void planificar(t_config, t_list*,t_list*,t_list*);
void pasarAReady(t_pcb*,t_list*);
void planificarFIFO(t_list*, int);

#endif
