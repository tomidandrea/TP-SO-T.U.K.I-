#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <string.h>


// TODO: esto si no se usa volarlo
typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    EXIT
} Estado;

typedef struct {
    char AX [4], BX [4], CX [4], DX [4];
	char EAX [8], EBX [8], ECX [8], EDX [8];
	char RAX [16], RBX [16], RCX [16], RDX [16];
} t_registros;

typedef struct {
    int pid;
    t_list* instrucciones;
    int pc;
    t_registros registros;
    Estado estado;
    char* motivo;
    // implementar los otros xd
} t_pcb;

t_pcb* crearPCB(t_list* listaInstrucciones);

void mandar_pcb_a_CPU();


#endif
