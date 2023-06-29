/*
 * visualizarEstructuras.h
 *
 *  Created on: May 24, 2023
 *      Author: utnso
 */

#ifndef SRC_UTILS_VISUALIZARESTRUCTURAS_H_
#define SRC_UTILS_VISUALIZARESTRUCTURAS_H_

#include<utils/general.h>

void mostrarRegistros(t_registros* registros);
void mostrarListaProcesos(t_list* lista);
char* lista_procesos_string(t_list* lista);
void logearInstrucciones(t_list* instrucciones, t_log* logger);
void mostrarListaSegmentos(tabla_segmentos tabla);
void mostrarTablaHuecos(tabla_segmentos tabla);


#endif /* SRC_UTILS_VISUALIZARESTRUCTURAS_H_ */
