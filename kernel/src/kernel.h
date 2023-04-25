#ifndef KERNEL_H_
#define KERNEL_H_

#include <readline/readline.h>
#include <pthread.h>
#include <listasPCB.h>
#include <planificacion.h>
#include <assert.h>
#include <utils/general.h>
#include <utils/sockets.h>


typedef struct{
	t_socket cliente;
	t_log* logger;
	t_list* lista;
	//char* server_name;
} hilo_consolas_args;

void atender_cliente(hilo_consolas_args* argumentos);

#endif
