#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<assert.h>
#include<unistd.h>
#include<netdb.h>

typedef struct t_instruccion {
	char* instruccion;
	char parametros[2][20];
} t_instruccion;

t_log* iniciar_logger(void);
t_config* iniciar_config (char*);

#endif
