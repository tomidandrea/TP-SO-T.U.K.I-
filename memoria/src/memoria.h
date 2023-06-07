/*
 * memoria.h
 *
 *  Created on: Apr 18, 2023
 *      Author: utnso
 */

#ifndef SRC_MEMORIA_H_
#define SRC_MEMORIA_H_

#include <utils/general.h>
#include <utils/sockets.h>

struct {
	int tamanio_memoria;
	int tamanio_segmento_0;
	int cantidad_segmentos;
	int retardo_memoria;
	int retardo_compactacion;
	char* algoritmo_asignacion;
} t_datosConfig;

void * espacio_contiguo;

#endif /* SRC_MEMORIA_H_ */
