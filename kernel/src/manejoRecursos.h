/*
 * manejoRecursos.h
 *
 *  Created on: Jun 19, 2023
 *      Author: utnso
 */

#ifndef SRC_MANEJORECURSOS_H_
#define SRC_MANEJORECURSOS_H_

#include <utils/general.h>
#include <utils/sockets.h>
#include <sincro.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <string.h>
#include <semaphore.h>

#define RECURSO_EXISTENTE 1
#define RECURSO_INEXISTENTE 0

typedef struct{
	t_pcb* proceso;
	int tiempo_sleep;
}io_contexto;

void inicializarRecursos();
void pasarAInstanciasEnteras();
int verificarRecursos(char* recurso);
int cantInstancias(char* recurso);
int indice(char* recurso);

void wait(t_pcb* proceso, char* recurso);
void ejecutarSignal(t_pcb* proceso, char* recurso);

io_contexto* inicializarIoContexto(t_pcb* proceso, int tiempo);
void ejecutarIO(io_contexto* contexto);
void bloquearYPasarAReady(io_contexto* contexto);

void crearColasDeBloqueados();
void aumentarInstancias(char* recurso);
void disminuirInstancias(char* recurso);

void desbloquearPrimerProceso(char* recurso);
void bloquear(t_pcb* proceso, char* recurso);

t_list* obtenerTodosProcesosBloqueados();
t_pcb* obtenerProcesoQueue(t_queue *self, int indice);

#endif /* SRC_MANEJORECURSOS_H_ */
