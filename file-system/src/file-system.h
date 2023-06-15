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
	uint32_t* puntero_directo;
	uint32_t* puntero_indirecto;

} t_fcb;


void crear_fcb(char* archivo, t_list*fcbs);
int existe_fcb(char*archivo, t_list*fcbs);
int compararNombres(t_fcb*fcb, char*archivo);

#endif /* SRC_FILE_SYSTEM_H_ */
