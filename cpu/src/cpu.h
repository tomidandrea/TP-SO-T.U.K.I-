/*
 * cpu.h
 *
 *  Created on: Apr 18, 2023
 *      Author: utnso
 */

#ifndef SRC_CPU_H_
#define SRC_CPU_H_

#include <string.h>
#include<commons/string.h>
#include <utils/general.h>
#include <utils/sockets.h>


typedef enum {
    CONTINUAR,
    FIN,
	DESALOJAR,
	BLOQUEAR

} estado_ejec;

typedef struct {
	char AX[4], BX[4], CX[4], DX[4];
	char EAX[8], EBX[8], ECX[8], EDX[8];
	char RAX[16], RBX[16], RCX[16], RDX[16];
} t_registros;

typedef struct {
    int pid;
    t_list* instrucciones;
    int pc;
    t_registros registros;
} t_pcb;

typedef struct {
	int numero;
	int desplazamiento;

} t_segmento;

t_pcb* inicializar_pcb();
t_pcb* recibirProceso(t_socket*socket_cliente);
estado_ejec realizar_ciclo_instruccion(t_pcb * pcb);
t_instruccion* fetch(t_list* instrucciones, uint32_t pc);
int decode(char* instruccion);
int requiere_memoria(char* instruccion);
estado_ejec execute(t_instruccion* instruccion_ejecutar, int pid);
void ejecutar_set(char* registro, char* valor);
int entra_en_registro(char*registro,char*valor);

#endif /* SRC_CPU_H_ */
