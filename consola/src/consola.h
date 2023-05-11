#ifndef CONSOLA_H_
#define CONSOLA_H_

#include<readline/readline.h>
#include<assert.h>
#include <utils/sockets.h>
#include <utils/general.h>
#include <parser.h>

void enviar_programa(t_list* instrucciones, int conexion);
void liberarEstructuras(void);

#endif
