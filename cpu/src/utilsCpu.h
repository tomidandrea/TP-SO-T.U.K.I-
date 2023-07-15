#ifndef SRC_UTILSCPU_H_
#define SRC_UTILSCPU_H_

#include <utils/sockets.h>
#include <utils/general.h>

t_pcb* recibir_proceso(int socket_cliente);
void recibir_variable(int* variable, t_buffer* buffer,int* desplazamiento);
void actualizar_registros_cpu(t_pcb *pcb, t_list*lista_registros);
void actualizar_registros_pcb(t_pcb *pcb);
void enviar_contexto(t_pcb* proceso, t_instruccion* instruccion_privilegiada, int conexion);
void escribir_memoria(int pid, u_int32_t direc_fisica,char* valor, int tamanio_valor);
char* leer_memoria(int pid, u_int32_t direc_fisica,int tamanio_a_leer);
void liberar_proceso(t_pcb* pcb);

#endif /* SRC_UTILSCPU_H_ */
