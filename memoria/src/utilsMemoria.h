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


// ---- INICIO DE MEMORIA ----

t_segmento* crear_t_segmento(int id, u_int32_t base, u_int32_t limite);
void inicializarEstructuras();
tabla_segmentos inicializarTablaHuecosLibres(int tamanioMemoria,int tamanioSegmento0);


// ----- SEGMENTACIÓN -----

// Envia la tabla de segmentos a kernel
void enviarSegmentosKernel(int socket_kernel, tabla_segmentos tablaSegmentos);

void enviarSegmentoCreado(t_socket socket_kernel, tabla_segmentos tabla_segmentos);

// Dado un pedido de creación de segmento y el algoritmo de asignación,
// retorna si hay hueco asignable y guarda el mismo en una variable global.
// Si no entra el pedido en ningún hueco, pero si en la suma de los mismos,
// retorna que debe compactarse.
int hayEspacio(t_pedido_segmento* pedido);

t_segmento* obtenerHuecoPorId(tabla_segmentos tabla_huecos, int huecoDisponible);
void removerHuecoPorId(tabla_segmentos tabla_huecos, int huecoDisponible);

// Se crea el segmento y se guarda en el hueco disponible
void crearSegmento(t_pedido_segmento* pedido);

// Retorna el hueco mayor o menor según el algoritmo de asignacion
t_segmento* obtenerHuecoSegunAlgoritmo(t_segmento*, t_segmento*, t_algoritmo_memoria);

u_int32_t obtenerTamanioSegmento(t_segmento*);
int obtenerIndiceSegmento(tabla_segmentos, int);

// Elimina el segmento y agrega el hueco que dejó a la lista de huecos
void eliminarSegmento (t_pedido_segmento*);

// ----- FIN DE PROCESO -----

void actualizarHuecos(tabla_segmentos tablaProceso);
void liberarEstructurasProceso(char* pid);

tabla_segmentos unificarTablas();
bool esMenorBase(void*, void*);
void compactar(t_pedido_segmento* pedido);

#endif /* SRC_UTILSMEMORIA_H_ */
