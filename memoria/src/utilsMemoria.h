/*
 * utilsMemoria.h
 *
 *  Created on: Jun 8, 2023
 *      Author: utnso
 */

#ifndef SRC_UTILSMEMORIA_H_
#define SRC_UTILSMEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <utils/sockets.h>
#include <utils/general.h>
#include <commons/collections/dictionary.h>

typedef enum {
    HAY_HUECO_ASIGNABLE,
	HAY_ESPACIO_AL_COMPACTAR,
	NO_HAY_HUECO_ASIGNABLE
}EstadoMemoria;

t_segmento* crear_t_segmento(int id, u_int32_t base, u_int32_t limite);
void inicializarEstructuras();
tabla_segmentos inicializarTablaHuecosLibres(int tamanioMemoria,int tamanioSegmento0);
void enviarSegmentosKernel(int socket_kernel, tabla_segmentos tablaSegmentos);
int hayEspacio(t_pedido_segmento* pedido);
void crearSegmento(t_pedido_segmento* pedido);
t_segmento* obtenerHuecoMenorTamanio(t_segmento*, t_segmento*);
t_segmento* obtenerHuecoMayorTamanio(t_segmento*, t_segmento*);
u_int32_t obtenerTamanioSegmento(t_segmento* segmento);
void eliminarSegmento (t_pedido_segmento*);
void enviarSegmentoCreado(t_socket socket_kernel, tabla_segmentos tabla_segmentos);

#endif /* SRC_UTILSMEMORIA_H_ */
