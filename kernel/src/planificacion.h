#ifndef PLANIF_H
#define PLANIF_H

#include <utils/general.h>
#include <utils/sockets.h>
#include <semaphore.h>
#include <pthread.h>
#include "utilsKernel.h"
#include <sincro.h>

void planificar();
void recibirDeCPU();
t_pcb* planificarFIFO();
t_pcb* planificarHRRN();
void agregarReady();
void mandar_pcb_a_CPU(t_pcb*);
t_contexto* actualizar_pcb(t_pcb*);
#endif
