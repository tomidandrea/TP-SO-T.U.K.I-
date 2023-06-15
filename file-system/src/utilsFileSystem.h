/*
 * utilsFileSystem.h
 *
 *  Created on: Jun 12, 2023
 *      Author: utnso
 */

#ifndef SRC_UTILSFILESYSTEM_H_
#define SRC_UTILSFILESYSTEM_H_

#include <utils/general.h>
#include <utils/sockets.h>
#include <commons/bitarray.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int mapearArchivo(void*espacio_memoria,void*path,size_t cantidad_bytes);
void recibo_parametros(t_socket socket_cliente,char** parametros);

#endif /* SRC_UTILSFILESYSTEM_H_ */
