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
#include <math.h>

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

typedef struct {
	int numero_segmento;
	int desplazamiento;
} direc_logica ;

t_instruccion* realizar_ciclo_instruccion(t_pcb * pcb);
t_instruccion* fetch(t_list* instrucciones, int pc);
void decode(char* instruccion);
int requiere_memoria(char* instruccion);
estado_ejec execute(t_instruccion* instruccion_ejecutar,t_pcb *pcb);
estado_ejec set_registro(char* registro, char* valor);
char* get_registro(char*registro);
estado_ejec ejecutar_mov_in(int pid, char* registro, char* direc, tabla_segmentos tabla_de_segmentos);
estado_ejec ejecutar_mov_out(int pid, char* direc,char* registro, tabla_segmentos tabla_de_segmentos);
estado_ejec ejecutar_mov(int pid,int tamanio_valor, direc_logica direcLogica, tabla_segmentos tabla_de_segmentos);
direc_logica crear_direc_logica(char* direc);
int verificar_num_segmento(int num_segmento,tabla_segmentos tabla_de_segmentos);
int no_produce_seg_fault(int pid, int desplazamiento,int tamanio_a_leer, t_segmento*segmento);
int tamanio_registro(char*registro);
int obtener_direc_fisica(direc_logica direcLogica,tabla_segmentos tabla_de_segmentos);



#endif /* SRC_CPU_H_ */
