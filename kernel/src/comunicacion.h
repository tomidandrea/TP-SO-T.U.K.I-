#ifndef SRC_COMUNICACION_H_
#define SRC_COMUNICACION_H_

#include <utils/general.h>
#include <utils/sockets.h>
#include <semaphore.h>

int escucharConsolas();
void mandar_pcb_a_CPU(t_pcb* proceso);
void avisar_fin_a_consola(t_socket socket_consola);
void avisar_fin_a_memoria(int pid);
//void finalizar_proceso(t_pcb* proceso);

void enviarAMemoria(int id_segmento, int tamanio_segmento);
void pedirTablaSegmentos();

#endif
