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

t_pcb* crearPCB(t_list* listaInstrucciones);

void inicializarSemoforos();
void crearEscucharCPU();
void crearEscucharConsolas();
void crearAgregarReady();
void crearPlanificar();
void liberarSemoforos();
void liberarMutex();


#endif
