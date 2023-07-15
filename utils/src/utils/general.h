#ifndef SRC_UTILS_GENERAL_H_
#define SRC_UTILS_GENERAL_H_

#include<utils/estructuras.h>
#include<sys/socket.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/string.h>
#include<commons/collections/queue.h>
#include<commons/string.h>
#include<utils/serializacion.h>

#define TAMANIO_OPERACION 15 //Tamaño maximo de operacion, así usamos malloc de este tamaño (14 + 1 por el \0)
#define CANT_IDENTIFICADORES (sizeof(tablaIdentificadores)/sizeof(t_identificador))

t_pcb* inicializar_pcb();
void liberar_pcb(t_pcb* pcb);
t_contexto* inicializar_contexto();

t_registros* inicializarRegistros();

/*
 * Funcion id: la usamos para obtener el Enum de la instruccion a partir de string instruccion
 * Usado en: CPU
 */
int id(char * instruccion);
/*
 * Funcion cantParametros: la usamos para obtener la cantidad de parametros de una instruccion
 * 		a partir de string instruccion
 */
int cantParametros(char *instruccion);
/*
 * Funcion listaAInstrucciones: a partir de una lista de strings de operaciones y parametros convertirmos
 * 		dichos strings en t_instruccion y luego devolvemos una lista de este tipo
 */
t_list* listaAInstrucciones(t_list*);

t_log* iniciar_logger(char* file, char *process_name, bool is_active_console, t_log_level level);
t_config* iniciar_config (char*);

t_instruccion* inicializar_instruccion(int cantidadParametros);
void liberar_instrucciones(t_list* instrucciones);
void liberar_instruccion(t_instruccion* instruccion);
char** inicializar_parametros(int cantidadParametros);
void liberar_parametros(char** parametros, int cantidadParametros);
/*
 * Funcion copiar: a partir de un string, lo copia a un nuevo char* haciendo el malloc
 * 		del tamaño exacto del string + 1 agregando un \0
 */
char* copiar(char* palabra);
void copiarRegistros(t_registros* registrosDestino, t_registros* registrosOrigen);
/*
 * Funcion contar: es generica, cuenta la cantidad de veces del caracter en la cadena
 */
int contar(char* cadena, char caracter);

int idAlgoritmo(char * algoritmo);

int obtenerIndiceSegmento(tabla_segmentos, int);
t_segmento* obtenerSegmentoPorId(tabla_segmentos tabla_segmentos, int id);

void liberar_contexto(t_contexto* contexto);

void removerSegmento0(tabla_segmentos tabla_seg);
void liberarTablaSegmentos(void* tablaProceso);
void liberarArchivo(t_archivo* archivo);

#endif

