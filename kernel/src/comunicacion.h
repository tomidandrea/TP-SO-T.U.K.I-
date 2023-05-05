#ifndef SRC_COMUNICACION_H_
#define SRC_COMUNICACION_H_

#include <utils/general.h>
#include <utils/sockets.h>
#include <semaphore.h>

int escucharConsolas();
int escucharCPU();
void mandar_pcb_a_CPU();

#endif
