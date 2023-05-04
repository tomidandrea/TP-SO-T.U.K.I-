#ifndef PLANIF_H
#define PLANIF_H

#include <utils.h>
#include <utils/general.h>
#include <utils/sockets.h>
#include <semaphore.h>
#include <pthread.h>


void planificar();
void pasarAReady();
void planificarFIFO();
void agregarReady();
#endif
