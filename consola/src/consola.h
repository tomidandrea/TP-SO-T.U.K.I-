#ifndef CONSOLA_H_
#define CONSOLA_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<commons/collections/list.h>
#include<assert.h>
#include<unistd.h>
#include<netdb.h>
#include <utils/sockets.h>
#include <utils/general.h>



t_config* iniciar_config (char* path);
t_log* iniciar_logger(void);

void logearInstrucciones(t_list* instrucciones, t_log* logger);


//TODO hacer parser.h y utils.h


void leerParametro(FILE *archivo, int cantParametros,char** parametros);

void parsear_instrucciones(char* path, t_list* codigo);

void enviar_programa(t_list* instrucciones, int conexion);



#endif
