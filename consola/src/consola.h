#ifndef CONSOLA_H_
#define CONSOLA_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<assert.h>
#include<unistd.h>
#include<netdb.h>

t_config* iniciar_config (char* path);
t_log* iniciar_logger(void);

typedef struct t_instruccion {
	char* instruccion;
	char parametros[2][20];
} t_instruccion;

void parsear_instruccion(t_instruccion* instruccion, FILE* archivo, t_log* logger);

#endif
