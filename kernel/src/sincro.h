#ifndef SRC_SINCRO_H_
#define SRC_SINCRO_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <semaphore.h>
#include <pthread.h>
#include <utils/general.h>
#include <planificacion.h>
#include <comunicacion.h>

void crearEscucharConsolas();
void crearAgregarReady();
void crearPlanificar();
void crearRecibirDeCPU();
void crearRecibirDeFS();

void inicializarSemoforos();
void liberarSemoforos();
void liberarMutex();

void pasarNewAReady();
void pasarAReady(t_pcb* proceso);
void pasarAExecute(t_pcb* proceso);
void agregarAlInicioDeReady(t_pcb* proceso);
t_pcb* removerPrimeroDeReady();
t_pcb* removerDeExecute();

#endif
