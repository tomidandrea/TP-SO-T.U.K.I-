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


/*
typedef struct {
    char AX [4], BX [4], CX [4], DX [4];
	char EAX [8], EBX [8], ECX [8], EDX [8];
	char RAX [16], RBX [16], RCX [16], RDX [16];
} t_registros;*/

t_pcb* crearPCB(t_list* listaInstrucciones);
t_socket crearConexionCPU();
void crearEscucharConsolas();
void crearAgregarReady();
void crearPlanificar();

void inicializarRecursos();
void inicializarSemoforos();
void liberarSemoforos();
void liberarMutex();

int verificarRecursos(char* recurso);
void wait(char* recurso);
void ejecutarSignal(char* recurso);
int indice(char* recurso);
int cantInstancias(char* recurso);
void crearColasDeBloqueados();
void aumentarInstancias(char* recurso);
void disminuirInstancias(char* recurso);
void desbloquearPrimerProceso(char* recurso);
void bloquear(char* recurso);
t_pcb* sacarDeCPU();
void agregarAlInicioDeReady(t_pcb* proceso);
void pasarAInstanciasEnteras();
#endif
