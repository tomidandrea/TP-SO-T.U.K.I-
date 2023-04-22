#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    EXIT
} Estado;

typedef struct {
    uint16_t AX,BX,CX,DX;
    uint16_t EAX, EBX, ECX, EDX;
    uint16_t RAX, RBX, RCX, RDX;
} t_registros;

typedef struct {
    int pid;
    t_list* instrucciones;
    int pc;
    t_registros registros;
    Estado estado;
    // implementar los otros xd
} t_pcb;

t_pcb* crearPCB(t_list* listaInstrucciones);

void mandar_pcb_a_CPU();


#endif
