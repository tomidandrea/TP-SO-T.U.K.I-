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


t_config* iniciar_config (char* path);
t_log* iniciar_logger(void);


typedef struct {
	char *operacion;
	int cantParametros;
} t_identificador;

typedef struct {
	char* instruccion;
	char** parametros;

} t_instruccion;

#define CANT_IDENTIFICADORES (sizeof(tablaIdentificadores)/sizeof(t_identificador))

int cantParametros(char *instruccion);

void leerParametro(FILE *archivo, int cantParametros,char** parametros);

void parsear_instrucciones(char* path, t_list* codigo);

#endif
