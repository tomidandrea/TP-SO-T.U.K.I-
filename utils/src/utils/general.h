/*
 * general.h
 *
 *  Created on: Apr 18, 2023
 *      Author: utnso
 */

#ifndef SRC_UTILS_GENERAL_H_
#define SRC_UTILS_GENERAL_H_

#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include<utils/serializacion.h>

typedef struct {
	char *operacion;
	int cantParametros;
} t_identificador;

typedef struct {
	char* instruccion;
	char **parametros;

} t_instruccion;

#define CANT_IDENTIFICADORES (sizeof(tablaIdentificadores)/sizeof(t_identificador))


void logearInstrucciones(t_list* instrucciones, t_log* logger);
int cantParametros(char *instruccion);
t_list* listaAInstrucciones(t_list* list);

t_log* iniciar_logger(char* file, char *process_name, bool is_active_console, t_log_level level);
t_config* iniciar_config (char*);

#endif /* SRC_UTILS_GENERAL_H_ */
