#ifndef KERNEL_H_
#define KERNEL_H_

#include <planificacion.h>
#include <readline/readline.h>
#include <pthread.h>
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


t_pcb* recibir_proceso(t_socket);
void cerrar(int signal);

#endif
