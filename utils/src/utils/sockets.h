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

// Utils cliente
t_socket iniciarConexion(t_config*, t_log*, char*, char*);
t_socket crear_conexion(char*, char*, t_log*);

void enviar_mensaje(char* mensaje, int socket_cliente);
char* recibir_mensaje(int socket_cliente, t_log* logger);

void enviar_paquete(t_paquete*, int);
void liberar_conexion(int);

//Utils server
t_socket iniciarServidor(t_config*, t_log*, char*);
t_socket iniciar_servidor(char* , t_log*);
t_socket esperar_cliente(int, t_log*);
int recibir_operacion(t_socket);
void* recibir_buffer(int*, int);
t_list* recibir_paquete(int);
t_pcb* inicializar_pcb();
t_contexto* recibir_contexto(int);
t_list* recibirTablaSegmentos(t_socket);
char* recibirPID(t_socket);
t_segmento* recibirSegmento(t_socket);
t_pedido_segmento* recibirPedidoSegmento(t_socket);
t_pedido_segmento* recibirPedidoDeleteSegment(t_socket);

#endif
