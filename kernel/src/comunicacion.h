#ifndef SRC_COMUNICACION_H_
#define SRC_COMUNICACION_H_

#include <utils/general.h>
#include <utils/sockets.h>
#include <semaphore.h>

int escucharConsolas();
void mandar_pcb_a_CPU(t_pcb* proceso);

#endif
