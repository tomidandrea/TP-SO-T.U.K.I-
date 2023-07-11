#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <string.h>
#include <utils/general.h>
#include <utils/sockets.h>
#include <planificacion.h>

/*
typedef struct {
    char AX [4], BX [4], CX [4], DX [4];
	char EAX [8], EBX [8], ECX [8], EDX [8];
	char RAX [16], RBX [16], RCX [16], RDX [16];
} t_registros;*/

t_pcb* crearPCB(t_list* listaInstrucciones, t_socket socket_consola);


t_temporal* iniciarTiempo();
t_temporal* pararTiempo(t_temporal* temporal);


t_contexto* actualizar_pcb(t_pcb*);

void solicitarCrearSegmento(int id, int tamanio, t_pcb* proceso);
void recibirCrearSegmento(int id, int tamanio, t_pcb* proceso);
void solicitarEliminarSegmento(int id, t_pcb* proceso);
void recibirEliminarsegmento(t_pcb* proceso);
void recibirTablaActualizada(t_pcb* proceso);

t_pcb* obtenerProcesoPorPID(int pid, t_list* procesos);
void obtenerProcesosReady(t_list* procesos);
void actualizarTablaProceso(char* pidString, tabla_segmentos tabla, t_list* procesos);
void actualizarTablasDeSegmentos(int conexionMemoria, t_pcb*);

#endif
