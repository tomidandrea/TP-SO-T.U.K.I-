#ifndef PLANIF_H
#define PLANIF_H

#include <utils.h>
#include <utils/general.h>
#include <utils/sockets.h>
#include <semaphore.h>
#include <pthread.h>


void planificar();
void pasarAReady();
t_pcb* planificarFIFO();
void agregarReady();
void mandar_pcb_a_CPU(t_pcb*);
void actualizar_pcb(t_pcb*);
#endif
