/*
 * manejoArchivos.h
 *
 *  Created on: Jun 25, 2023
 *      Author: utnso
 */

#ifndef SRC_MANEJOARCHIVOS_H_
#define SRC_MANEJOARCHIVOS_H_

#include <utils/general.h>
#include <utils/sockets.h>
#include <sincro.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <string.h>
#include <semaphore.h>


t_archivo_global* inicializarArchivoGlobal(char * nombre);
t_archivo* inicializarArchivo(char * nombre);
int estaAbiertoElArchivo(char * nombre);
t_archivo_global* archivoGlobalQueSeLlama(char* nombre);
t_archivo* archivoQueSeLlama(char* nombre, tabla_archivos archivosAbiertos);
void abrirArchivoEnFS(char* nombre);
void crearArchivoEnFS(char* nombre);
void truncar_archivo(char* nombre, int tamanio);
void leer_archivo(int pid, t_archivo* archivo, u_int32_t direc_fisica, int cant_bytes);
void escribir_archivo(int pid, t_archivo* archivo, u_int32_t direc_fisica, int cant_bytes);
void actualizar_puntero(t_pcb* proceso, t_archivo* archivoProceso, int puntero);
void bloquearPorFS(t_pcb* proceso, char* motivo);
void bloquearEnColaDeArchivo(t_archivo_global* archivo, t_pcb* proceso);
void desbloquearDeColaDeArchivo(t_archivo_global* archivo);
void desbloquearDeEsperaDeFS();
void liberarArchivoGlobal(t_archivo_global* archivo);
void liberarArchivo(t_archivo* archivo);

#endif /* SRC_MANEJOARCHIVOS_H_ */
