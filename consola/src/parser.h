#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include <utils/general.h>

void leerParametro(FILE *archivo, int cantParametros,char** parametros);
void parsear_instrucciones(char* path, t_list* codigo);

#endif /* SRC_PARSER_H_ */
