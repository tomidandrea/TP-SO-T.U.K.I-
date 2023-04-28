#ifndef KERNEL_H_
#define KERNEL_H_

#include <planificacion.h>
#include <readline/readline.h>
#include <pthread.h>
#include <listasPCB.h>
#include <assert.h>
#include <utils/general.h>
#include <utils/sockets.h>
#include <semaphore.h>



typedef struct{
	t_socket cliente;
	t_log* logger;
	t_list* lista;
	//char* server_name;
} hilo_consolas_args;

//TODO SACAR CREO
void atender_cliente(hilo_consolas_args* argumentos);

t_pcb* recibir_proceso(t_socket);

#endif
