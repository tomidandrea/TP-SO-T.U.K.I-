/*
 * utilsFileSystem.h
 *
 *  Created on: Jun 12, 2023
 *      Author: utnso
 */

#ifndef SRC_UTILSFILESYSTEM_H_
#define SRC_UTILSFILESYSTEM_H_

#include <utils/sockets.h>
#include <commons/bitarray.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

t_bitarray* mapear_bitmap(size_t cant_bits, size_t cant_bytes, char*path);
void mapear_bloques(void*bloques,char*path);
void* mapearArchivo(void*path,int*fd);
void set_archivo_bitmap(char*path,size_t cant_bits);
void inicializar_bitarray(t_bitarray*bitarray,size_t cant_bits);
void recibo_parametros(t_socket socket_cliente,char** parametros);

#endif /* SRC_UTILSFILESYSTEM_H_ */
