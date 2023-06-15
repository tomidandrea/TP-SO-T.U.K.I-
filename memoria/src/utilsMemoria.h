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

void inicializarEstructuras();
void enviarSegmentosKernel(int socket_kernel);

#endif /* SRC_UTILSMEMORIA_H_ */
