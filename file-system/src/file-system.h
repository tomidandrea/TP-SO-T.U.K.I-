/*
 * file-system.h
 *
 *  Created on: Apr 18, 2023
 *      Author: utnso
 */

#ifndef SRC_FILE_SYSTEM_H_
#define SRC_FILE_SYSTEM_H_

#include <utilsFileSystem.h>


typedef struct {
	char* nombre;
	int tamanio;
	uint32_t puntero_directo;
	uint32_t puntero_indirecto;
	t_config*config;

} t_fcb;

// fcb
void crear_fcb(char* archivo, t_list*fcbs, char*path);
int existe_fcb(char*archivo, t_list*fcbs);
void liberar_fcb(t_fcb*fcb);
t_fcb* get_fcb(char*archivo, t_list*fcbs);

// truncado
bool truncar_archivo(char*, int, size_t, t_list*, t_bitarray*,FILE*) ;
void agregar_bloques(t_fcb*fcb,size_t cant_bloques,t_bitarray* bitmap,FILE*archivo_bloques);
void asignar_bloques_a_fcb(uint32_t bloques_asignados[],size_t cant_bloques,t_fcb*fcb,t_bitarray*bitmap,FILE*archivo_bloques);
void asignar_bloques_indirectos(t_fcb*fcb,uint32_t bloques_asignados[],size_t cant_bloques,size_t cant_bloques_indirectos, int desde, FILE* archivo_bloques);
void escribir_bloques_en_bloque_de_punteros(uint32_t puntero_indirecto,uint32_t bloques[],size_t cant_bloques,FILE* archivo_bloques);
void liberar_bloques(t_fcb*fcb,size_t cant_bloques_a_liberar,size_t cant_bloques_indirectos_actual,t_bitarray*bitmap, FILE*archivo_bloques);
void leer_bloques_a_liberar(uint32_t puntero_indirecto,size_t cant_bloques_,uint32_t bloques_a_liberar[],size_t cant_bloques_indirectos_actual, FILE*archivo_bloques);

#endif /* SRC_FILE_SYSTEM_H_ */
