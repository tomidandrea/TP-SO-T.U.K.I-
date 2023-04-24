#ifndef KERNEL_H_
#define KERNEL_H_

#include <readline/readline.h>
#include <assert.h>
#include <utils/general.h>
#include <utils/sockets.h>
#include <pthread.h>
#include <listasPCB.h>
#include <planificacion.h>


typedef struct{
	t_socket cliente;
	t_log* logger;
	t_list* lista;
	//char* server_name;
} hilo_consolas_args;

typedef struct{
	t_config* config;
	t_list* procesosNew;
	t_list* procesosReady;
	t_list* procesosExecute;
}hilo_planificador_args;


/*typedef struct t_instruccion {
	char* instruccion;
	char parametros[2][20];
} t_instruccion;*/


void atender_cliente(hilo_consolas_args* argumentos);

#endif
