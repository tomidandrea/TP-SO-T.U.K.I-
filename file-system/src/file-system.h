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
void crear_fcb(char* archivo, char*path_archivo);
bool existe_fcb(char*archivo,char*path);
void liberar_fcb(t_fcb*fcb);
t_fcb* get_fcb(char*archivo, char*path_fcb);
void actualizar_archivo_fcb(t_fcb*fcb);

// truncado
bool truncar(char*, int, char*, t_bitarray*,FILE*) ;
bool agregar_bloques(t_fcb*,size_t, size_t, t_bitarray*, FILE*);
bool condicion_para_agregar_bloque_de_punteros(t_fcb*fcb, size_t cant_bloques_a_agregar);
bool asignar_bloques_a_fcb(uint32_t [],size_t, size_t, t_fcb*, t_bitarray*, FILE*);
bool asignar_bloques_indirectos(t_fcb*,uint32_t [],size_t,size_t, size_t, int, FILE*);
bool escribir_bloques_en_bloque_de_punteros(uint32_t, uint32_t[],size_t, size_t, FILE*);
bool liberar_bloques(t_fcb*, size_t, size_t, t_bitarray*, FILE*);
void liberar_bloque_directo (t_fcb*fcb,t_bitarray*bitmap);
bool leer_bloques_a_liberar(uint32_t, size_t, uint32_t[], size_t, FILE*);


#endif /* SRC_FILE_SYSTEM_H_ */
