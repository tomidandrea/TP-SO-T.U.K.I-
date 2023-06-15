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

#define RECURSO_EXISTENTE 1
#define RECURSO_INEXISTENTE 0

typedef struct{
	t_pcb* proceso;
	int tiempo_sleep;
}io_contexto;

/*
typedef struct {
    char AX [4], BX [4], CX [4], DX [4];
	char EAX [8], EBX [8], ECX [8], EDX [8];
	char RAX [16], RBX [16], RCX [16], RDX [16];
} t_registros;*/

t_pcb* crearPCB(t_list* listaInstrucciones, t_socket socket_consola);
t_socket crearConexionCPU();


t_temporal* iniciarTiempo();
t_temporal* pararTiempo(t_temporal* temporal);

void inicializarRecursos();


void ejecutarIO(io_contexto* contexto);
void bloquearYPasarAReady(io_contexto* contexto);
int verificarRecursos(char* recurso);
void wait(t_pcb* proceso, char* recurso);
void ejecutarSignal(t_pcb* proceso, char* recurso);
int indice(char* recurso);
int cantInstancias(char* recurso);
void crearColasDeBloqueados();
void aumentarInstancias(char* recurso);
void disminuirInstancias(char* recurso);
void desbloquearPrimerProceso(char* recurso);
void bloquear(t_pcb* proceso, char* recurso);
void pasarAInstanciasEnteras();
io_contexto* inicializarIoContexto(t_pcb* proceso, int tiempo);
void solicitarCrearSegmento(int id, int tamanio, t_pcb* proceso);
void recibirCrearSegmento(int id, int tamanio, t_pcb* proceso);
void eliminarSegmento(int id, t_pcb* proceso);
void recibirTablaActualizada(t_pcb* proceso);

#endif
