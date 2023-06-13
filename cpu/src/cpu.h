/*
 * cpu.h
 *
 *  Created on: Apr 18, 2023
 *      Author: utnso
 */

#ifndef SRC_CPU_H_
#define SRC_CPU_H_

#include <stdio.h>
#include <string.h>

#include "utilsCpu.h"

/*
typedef struct {
	char AX[4], BX[4], CX[4], DX[4];
	char EAX[8], EBX[8], ECX[8], EDX[8];
	char RAX[16], RBX[16], RCX[16], RDX[16];
} t_registros;*/

/* alternativa con vector de strings para cada tamaño (4,8 y 16)
typedef struct {
	char[4][4] tamanio_4;
	char[4][8] tamanio_8;
	char[4][16] tamanio_16;
} t_registros;
*/
/*
typedef struct {
    int pid;
    t_list* instrucciones;
    int pc;
    t_registros* registros;
} t_pcb;*/
/*
typedef struct{
	int numero;
	int desplazamiento;
} t_segmento;*/

t_instruccion* realizar_ciclo_instruccion(t_pcb * pcb);
t_instruccion* fetch(t_list* instrucciones, uint32_t pc);
int decode(char* instruccion);
int requiere_memoria(char* instruccion);
estado_ejec execute(t_instruccion* instruccion_ejecutar,t_pcb *pcb);
estado_ejec ejecutar_set(char* registro, char* valor);

#endif /* SRC_CPU_H_ */
