#ifndef KERNEL_H_
#define KERNEL_H_

#include<readline/readline.h>
#include<assert.h>
#include <utils/general.h>
#include <utils/sockets.h>
#include<pthread.h>
#include <planificacion.h>

typedef struct{
	t_socket cliente;
	t_log* logger;
	t_list* lista;
	//char* server_name;
} thread_args;

/*typedef struct t_instruccion {
	char* instruccion;
	char parametros[2][20];
} t_instruccion;*/


void atender_cliente(thread_args* argumentos);

#endif
