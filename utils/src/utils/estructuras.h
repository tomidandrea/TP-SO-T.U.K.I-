#ifndef SRC_UTILS_ESTRUCTURAS_H_
#define SRC_UTILS_ESTRUCTURAS_H_

#include<signal.h>
#include<unistd.h>
#include<netdb.h>

#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<commons/temporal.h>

typedef int t_socket;

typedef struct {
	char *operacion;
	int cantParametros;
	int id;
} t_identificador;

typedef struct {
	char *operacion;
	int id;
} t_algoritmo;

typedef enum {
	FIRST_FIT,
	BEST_FIT,
	WORST_FIT
} t_algoritmo_memoria;

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
	EXT,
	CREAR_ARCHIVO,
	SEG_FAULT
} operacion;

typedef struct {
	int pid;
	int id_segmento;
	u_int32_t tamanio;
} t_pedido_segmento;

typedef struct {
	int id;
	u_int32_t base;
	u_int32_t limite;
} t_segmento;

typedef struct {
	char* nombre;
	int puntero;
} t_archivo;

typedef struct {
	char* nombre;
	t_queue* cola;
} t_archivo_global;

typedef t_list* tabla_segmentos;
typedef t_list* tabla_archivos;

typedef struct {
    int pid;
    t_list* instrucciones;
    int pc;
    t_registros* registros;
    tabla_segmentos tablaSegmentos;
    tabla_archivos archivosAbiertos;
    operacion motivo;
    t_temporal* tiempoEnReady;
	t_temporal* tiempoCPU;
	double estimadoAnterior;
	double ratio;
	t_socket socket_consola;
} t_pcb;

typedef struct {
    int pid;
    int pc;
    t_registros* registros;
    operacion motivo;
    char** parametros;
    int cantidadParametros;
    tabla_segmentos tablaSegmentos;
    u_int32_t direc_fisica;
} t_contexto;


#endif
