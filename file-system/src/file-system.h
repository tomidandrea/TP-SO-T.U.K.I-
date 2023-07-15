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
	//t_config*archivo;

} t_fcb;

// fcb
void crear_fcb(char* archivo, char*path_archivo);
bool existe_fcb(char*archivo,char*path);
void liberar_fcb(t_fcb*fcb);
t_fcb* get_fcb(char*archivo, char*path_directorio);
void actualizar_archivo_fcb(t_fcb*fcb,char*path_directorio);

// truncado aumentar
bool truncar(char*, int, char*, t_bitarray*,FILE*) ;
bool agregar_bloques(t_fcb*,size_t, size_t, t_bitarray*, FILE*);
bool condicion_para_agregar_bloque_de_punteros(t_fcb*fcb, size_t cant_bloques_a_agregar);
bool solo_tiene_un_bloque_asignado(t_fcb*fcb);
bool asignar_bloques_a_fcb(uint32_t [],size_t, size_t, t_fcb*, t_bitarray*, FILE*);
bool asignar_bloques_indirectos(t_fcb*,uint32_t [],size_t,size_t, size_t, int, FILE*);
bool escribir_bloques_en_bloque_de_punteros(t_fcb*fcb, uint32_t[],size_t, size_t, FILE*);
//truncado achicar
bool liberar_bloques(t_fcb*, size_t, size_t, t_bitarray*, FILE*);
void liberar_bloque_directo (t_fcb*fcb,t_bitarray*bitmap);
bool obtener_bloques_del_bloque_de_punteros(t_fcb*fcb, size_t, uint32_t[], size_t, FILE*);

//leer Archivo
bool leer_archivo(int pid,char*,char*,int, uint32_t, int, FILE*);
char* leer_dato_en_archivo_de_bloques(t_fcb*fcb,uint32_t bloques_fs[],uint32_t bloques_locales[],int puntero,int cant_bytes,FILE*archivo_bloques);
int minimo(int x,int y);

//escribir archivo
bool escribir_archivo(int pid, char* nombreArchivo,char*path_directorio,int puntero, uint32_t direc_fisica, int cant_bytes,FILE*archivo_bloques);
bool escribir_dato_en_archivo_de_bloques(t_fcb*fcb,char*dato_a_escribir,uint32_t bloques_fs[],uint32_t bloques_locales[],int puntero,int cant_bytes,FILE*archivo_bloques);

//para ambos (leer y escribir)
bool obtener_bloques_del_fs_a_acceder(t_fcb*fcb,size_t cant_bloques,uint32_t bloques_locales[], uint32_t bloques_fs[],uint32_t bloque_inicio_local, FILE*archivo_bloques);

#endif /* SRC_FILE_SYSTEM_H_ */
