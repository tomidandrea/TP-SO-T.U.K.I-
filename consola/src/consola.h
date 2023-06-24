#ifndef CONSOLA_H_
#define CONSOLA_H_

#include<readline/readline.h>
#include<assert.h>
#include <utils/sockets.h>
#include <utils/general.h>
#include <utils/visualizarEstructuras.h>
#include <parser.h>

void enviar_programa(t_list* instrucciones, int conexion);
void liberarEstructuras(t_socket);

#endif
