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
#include<commons/string.h>
#include<utils/serializacion.h>

#define TAMANIO_OPERACION 15 //Tamaño maximo de operacion, así usamos malloc de este tamaño (14 + 1 por el \0)

typedef struct {
	char *operacion;
	int cantParametros;
	int id;
} t_identificador;

typedef struct {
	char* instruccion;
	char **parametros;

} t_instruccion;

typedef struct {
	char AX[4], BX[4], CX[4], DX[4];
	char EAX[8], EBX[8], ECX[8], EDX[8];
	char RAX[16], RBX[16], RCX[16], RDX[16];
} t_registros;

// TODO: esto si no se usa volarlo, Aclaracion: Estaba en utils del kernel, lo movi a utils
typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    EXIT
} Estado;

typedef enum {
	NUEVO,
    CONTINUAR,
	ERROR,
    FIN,
} estado_ejec;

typedef enum {
    SET,
    MOV_OUT,
    WAIT,
    IO,
	SIGNAL,
	MOV_IN,
	F_OPEN,
	YIELD,
	F_TRUNCATE,
	F_SEEK,
	CREATE_SEGMENT,
	F_WRITE,
	F_READ,
	DELETE_SEGMENT,
	F_CLOSE,
	EXT
} operacion;
typedef struct {
    int pid;
    t_list* instrucciones;
    int pc;
    t_registros* registros;
    operacion motivo;
    // implementar los otros xd
    //TODO JARWI VER ESTE TLIST
    // verificar si conviene mantener este t_pcb para los recursos o hacer otro
    //
    t_list* recursos;
    t_list* instanciasDeRecursos;
} t_pcb;

#define CANT_IDENTIFICADORES (sizeof(tablaIdentificadores)/sizeof(t_identificador))



t_pcb* inicializar_pcb();
void logearInstrucciones(t_list* instrucciones, t_log* logger);

t_registros* inicializarRegistros();

int id(char * instruccion);
int cantParametros(char *instruccion);
t_list* listaAInstrucciones(t_list*);

t_log* iniciar_logger(char* file, char *process_name, bool is_active_console, t_log_level level);
t_config* iniciar_config (char*);

void liberar_instrucciones(t_list* instrucciones);
void liberar_instruccion(t_instruccion* instruccion);
void liberar_parametros(char** parametros, int cantidadParametros);
char* copiar(char* palabra); //hacemos malloc aca dentro

#endif /* SRC_UTILS_GENERAL_H_ */
