/*
 * serializacion.c
 *
 *  Created on: Apr 9, 2023
 *      Author: utnso
 */
#include "serializacion.h"

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}


t_paquete* crear_paquete(op_code cod_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = cod_op;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void agregar_valor_estatico(t_paquete* paquete, int* valor)
{
	/*printf("Dentro de la funcion\n");
	printf("Valor %d\n", *valor);
	printf("Sizeof %d\n", sizeof(*valor));*/
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(*valor));
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, sizeof(*valor));

    paquete->buffer->size += sizeof(*valor);
}

void agregar_valor_uint(t_paquete* paquete, u_int32_t* valor)
{
	/*printf("Dentro de la funcion\n");
	printf("Valor %d\n", *valor);
	printf("Sizeof %d\n", sizeof(*valor));*/
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(*valor));
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, sizeof(*valor));

    paquete->buffer->size += sizeof(*valor);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
