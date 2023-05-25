#ifndef SRC_UTILSCPU_H_
#define SRC_UTILSCPU_H_

#include <utils/sockets.h>
#include <utils/general.h>

t_pcb* recibir_proceso(int socket_cliente);
void recibir_variable(int* variable, t_buffer* buffer,int* desplazamiento);
void actualizar_registros_cpu(t_pcb *pcb, t_list*lista_registros);
void actualizar_registros_pcb(t_pcb *pcb);
void enviar_contexto(t_pcb* proceso, t_instruccion* instruccion_privilegiada, int conexion);

#endif /* SRC_UTILSCPU_H_ */
