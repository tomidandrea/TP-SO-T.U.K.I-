/*
 * utilsFileSystem.h
 *
 *  Created on: Jun 12, 2023
 *      Author: utnso
 */

#ifndef SRC_UTILSFILESYSTEM_H_
#define SRC_UTILSFILESYSTEM_H_

#include <utils/sockets.h>
#include <utils/general.h>
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
char* crear_path_archivo(char*nombre_archivo,char*path_directorio);
t_bitarray* mapear_bitmap(size_t cant_bytes, FILE*archivo_bitmap);
void* mapearArchivo(FILE*archivo,size_t tamanio);

// bitarray
void inicializar_bitarray(t_bitarray*bitarray);
void mostrar_bitarray(t_bitarray*bitarray);
void setear_n_primeros_bits_en_bitarray(t_bitarray*bitarray,size_t cant_bits, uint32_t indices_bits_asignados[]);
bool se_asignaron_todos_los_bits(uint32_t indices_bits_asignados[],size_t cant_bits);
void clean_n_bits_bitarray(t_bitarray* bitarray,size_t cant_bits, uint32_t indices_bits_a_limpiar[]);

// conexiones, serializacion/deserializacion
char* recibirNombreArchivo(void* buffer, int* desplazamiento);
void recibirLeerOEscribir(void* buffer, int* desplazamiento, int* puntero, u_int32_t* direc_fisica, int* cant_bytes);
bool enviar_dato_a_escribir_a_memoria(int pid, char*dato_leido,uint32_t direc_fisica);
char*solicitar_leer_dato_a_memoria(int pid, uint32_t direc_fisica,int cant_bytes);

#endif /* SRC_UTILSFILESYSTEM_H_ */
