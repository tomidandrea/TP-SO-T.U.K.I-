#ifndef SRC_UTILS_SOCKETS_H_
#define SRC_UTILS_SOCKETS_H_

#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<utils/serializacion.h>
#include<utils/general.h>

#include<utils/visualizarEstructuras.h>

typedef int t_socket;

// Utils cliente
t_socket crear_conexion(char*, char*, t_log*);
void enviar_mensaje(char*, int);
void enviar_paquete(t_paquete*, int);
void liberar_conexion(int);

//Utils server
t_socket iniciar_servidor(char* , t_log*);
t_socket esperar_cliente(int, t_log*);
int recibir_operacion(int);
void* recibir_buffer(int*, int);
void recibir_mensaje(int, t_log*);
t_list* recibir_paquete(int);
t_pcb* inicializar_pcb();
t_pcb* recibir_contexto(int);

#endif
