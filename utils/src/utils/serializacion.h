#ifndef SRC_UTILS_SERIALIZACION_H_
#define SRC_UTILS_SERIALIZACION_H_

#include<stdio.h>
#include<stdlib.h>
#include <string.h>

typedef enum
{
	MENSAJE,
	PROGRAMA,
	PROCESO,
	CONTEXTO,
	ESCRIBIR,
	LEER,
	CREACION_EXITOSA,
	OUT_OF_MEMORY,
	LIMITE_SEGMENTOS_SUPERADO,
	TABLA_SEGMENTOS,
	CREATE_SEGMENT_OP,
	DELETE_SEGMENT_OP,
	FIN_PROCESO
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;
/*
typedef struct {
    char AX [4], BX [4], CX [4], DX [4];
	char EAX [8], EBX [8], ECX [8], EDX [8];
	char RAX [16], RBX [16], RCX [16], RDX [16];
} t_registros;*/

void* serializar_paquete(t_paquete*, int);
t_paquete* crear_paquete(op_code);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void agregar_valor_estatico(t_paquete* paquete, int* valor);
void agregar_valor_uint(t_paquete* paquete, u_int32_t* valor);
void eliminar_paquete(t_paquete* paquete);

#endif /* SRC_UTILS_SERIALIZACION_H_ */
