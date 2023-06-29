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
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// levantar archivos y mapeo
FILE* levantarArchivo(char*path,size_t cant_bytes);
t_bitarray* mapear_bitmap(size_t cant_bytes, FILE*archivo_bitmap);
void* mapearArchivo(FILE*archivo,size_t tamanio);

// bitarray
void inicializar_bitarray(t_bitarray*bitarray);
void setear_n_primeros_bits_en_bitarray(t_bitarray*bitarray,size_t cant_bits, uint32_t indices_bits_asignados[]);
bool se_asignaron_todos_los_bits(uint32_t indices_bits_asignados[],size_t cant_bits);
void clean_n_bits_bitarray(t_bitarray* bitarray,size_t cant_bits, uint32_t indices_bits_a_limpiar[]);

// conexiones, serializacion/deserializacion
void recibo_parametros(t_socket socket_cliente,char** parametros);

#endif /* SRC_UTILSFILESYSTEM_H_ */
