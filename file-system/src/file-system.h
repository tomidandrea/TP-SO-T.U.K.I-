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


void crear_fcb(char* archivo, t_list*fcbs, char*path);
int existe_fcb(char*archivo, t_list*fcbs);
void liberar_fcb(t_fcb*fcb);
t_fcb* get_fcb(char*archivo, t_list*fcbs);
bool truncar_archivo(char*, int, size_t, t_list*, t_bitarray*) ;
void agregar_bloques(t_fcb*fcb,size_t cant_bloques,t_bitarray* bitmap);
bool se_asignaron_todos_los_bloques(uint32_t bloques_asignados[],size_t cant_bloques);
void asignar_bloques_a_fcb(uint32_t bloques_asignados[],t_fcb*fcb,t_bitarray*bitmap);
uint32_t asignar_bloque_de_punteros(t_bitarray*bitmap);
#endif /* SRC_FILE_SYSTEM_H_ */
