#ifndef PLANIF_H
#define PLANIF_H

#include <utils.h>

#include <utils/general.h>
//TODO sacar
#include <utils/general.h>
#include <utils/sockets.h>
#include <semaphore.h>
#include <pthread.h>

void planificar();
void pasarAReady(t_pcb* pcb);
void planificarFIFO();
//TODO APARTIR DE ACA SE ELIMIÑIA TO
void escucharConsolas(t_socket server_fd);
void agregarReady();
#endif
