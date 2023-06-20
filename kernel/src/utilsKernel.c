#include "utilsKernel.h"

#include <comunicacion.h>

int processID=0;
uint32_t BLOQUEADO = 0;
uint32_t CONTINUAR_EJECUTANDO = 1;
extern t_config* config;
extern t_log* logger;
extern t_socket conexionCPU;
extern t_socket conexionMemoria;

extern sem_t sem_new_a_ready, sem_ready, sem_grado_multiprogramacion, sem_recibir, sem_execute;
extern pthread_mutex_t mutex_procesos_new;
extern pthread_mutex_t mutex_procesos_ready;
extern pthread_mutex_t mutex_procesos_execute;

t_socket crearConexionCPU(){
	char* ip = config_get_string_value(config,"IP_CPU");
	char* puerto = config_get_string_value(config,"PUERTO_CPU");
	t_socket conexion = crear_conexion(ip, puerto, logger);
	return conexion;
}

t_socket crearConexionMemoria(){
	char* ip = config_get_string_value(config,"IP_MEMORIA");
	char* puerto = config_get_string_value(config,"PUERTO_MEMORIA");
	t_socket conexion = crear_conexion(ip, puerto, logger);
	return conexion;
}


t_pcb* crearPCB(t_list* listaInstrucciones, t_socket socket_consola){

	t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->pid = ++processID;
    pcb->pc = 0;
    pcb->socket_consola = socket_consola;
    pcb->instrucciones = list_duplicate(listaInstrucciones);
    pcb->registros = inicializarRegistros();
    pcb->estimadoAnterior = config_get_double_value(config,"ESTIMACION_INICIAL");
    pcb->tiempoEnReady = iniciarTiempo();
    temporal_stop(pcb->tiempoEnReady);
    pcb->tiempoCPU = iniciarTiempo();
    temporal_stop(pcb->tiempoCPU);

    strcpy(pcb->registros->AX, "0");
    strcpy(pcb->registros->BX, "0");
    strcpy(pcb->registros->CX, "0");
    strcpy(pcb->registros->DX, "0");
    strcpy(pcb->registros->EAX, "0");
    strcpy(pcb->registros->EBX, "0");
    strcpy(pcb->registros->ECX, "0");
	strcpy(pcb->registros->EDX, "0");
	strcpy(pcb->registros->RAX, "0");
	strcpy(pcb->registros->RBX, "0");
	strcpy(pcb->registros->RCX, "0");
	strcpy(pcb->registros->RDX, "0");

	pedirTablaSegmentos(pcb->pid);
	int cod_op = recibir_operacion(conexionMemoria);
	if(cod_op == TABLA_SEGMENTOS){
//		t_list* lista = recibirTablaSegmentos(conexionMemoria);
//		pcb->tablaSegmentos = list_duplicate(lista);
		pcb->tablaSegmentos = recibirTablaSegmentos(conexionMemoria);

	}else
		log_error(logger, "Recibiste cualquier cosa pa");

    return pcb;
}

t_contexto* actualizar_pcb(t_pcb* proceso) {
	if(conexionCPU != -1){
				t_contexto* contexto;
				int cod_op = recibir_operacion(conexionCPU);
				if(cod_op == CONTEXTO) {

					contexto = recibir_contexto(conexionCPU);

					temporal_stop(proceso->tiempoCPU);
					// actualizo proceso con lo q viene del pcb (PC y registros)
					log_info(logger, "Recibo contexto pa - PID:%d", contexto->pid);

					proceso->pid = contexto->pid;
					proceso->pc = contexto->pc;
					proceso->motivo = contexto->motivo;

					copiarRegistros(proceso->registros, contexto->registros);
					/*strcpy(proceso->registros->AX, contexto->registros->AX);
					strcpy(proceso->registros->BX, contexto->registros->BX);
					strcpy(proceso->registros->CX, contexto->registros->CX);
					strcpy(proceso->registros->DX, contexto->registros->DX);
					strcpy(proceso->registros->EAX, contexto->registros->EAX);
					strcpy(proceso->registros->EBX, contexto->registros->EBX);
					strcpy(proceso->registros->ECX, contexto->registros->ECX);
					strcpy(proceso->registros->EDX, contexto->registros->EDX);
					strcpy(proceso->registros->RAX, contexto->registros->RAX);
					strcpy(proceso->registros->RBX, contexto->registros->RBX);
					strcpy(proceso->registros->RCX, contexto->registros->RCX);
					strcpy(proceso->registros->RDX, contexto->registros->RDX);*/


				} else {
					log_error(logger,"No me llego un proceso");
				}
			return contexto;
		}
	log_error(logger, "Se rompio la conexion con CPU");
	exit(1);
}

t_temporal* iniciarTiempo(){
	t_temporal* tiempo;
	tiempo = temporal_create();
	return tiempo;
}

t_temporal* pararTiempo(t_temporal* temporal){
	temporal_stop(temporal);
	temporal_destroy(temporal);
	t_temporal* nuevoTemporal;
	nuevoTemporal = temporal_create();
	temporal_stop(temporal);
	return nuevoTemporal;
}


void solicitarCrearSegmento(int id, int tamanio, t_pcb* proceso) {
	t_paquete *paquete = crear_paquete(CREATE_SEGMENT_OP);

		agregar_valor_estatico(paquete, &(proceso->pid));
		agregar_valor_estatico(paquete, &(id));
		agregar_valor_estatico(paquete, &(tamanio));
		// El tamanio es el limite
		enviar_paquete(paquete,conexionMemoria);
		eliminar_paquete(paquete);
}

void recibirCrearSegmento(int id, int tamanio, t_pcb* proceso) {
	int cod = recibir_operacion(conexionMemoria);
	switch(cod) {
	case CREACION_EXITOSA:
		t_segmento* segmento = malloc(sizeof(t_segmento));
		segmento->id = id;
		//recv(conexionMemoria, &segmento->base, sizeof(int) , MSG_WAITALL);
		segmento->limite = segmento->base + tamanio;
		//crearSegmento(proceso->tablaSegmentos, segmento) agrega el segmento en la tabla
		free(segmento);
		mandar_pcb_a_CPU(proceso);
		sem_post(&sem_recibir);
		break;
	case OUT_OF_MEMORY:
		avisar_fin_a_consola(proceso->socket_consola);
		sem_post(&sem_grado_multiprogramacion);
		log_info(logger, "Finaliza el proceso %d - Motivo: OUT_OF_MEMORY", proceso->pid);
		log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", proceso->pid);
		liberar_pcb(proceso);
		break;
	case LIMITE_SEGMENTOS_SUPERADO: //todo: mejorar esto, con solo un case de error de creacion reicibiendo el motivo x socket
		avisar_fin_a_consola(proceso->socket_consola);
		sem_post(&sem_grado_multiprogramacion);
		log_info(logger, "Finaliza el proceso %d - Motivo: LIMITE_SEGMENTOS_SUPERADO", proceso->pid);
		log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", proceso->pid);
		liberar_pcb(proceso);
		break;
	case TABLA_SEGMENTOS: //TODO: case temporal, despues borrar
		proceso->tablaSegmentos = recibirTablaSegmentos(conexionMemoria);
		log_info(logger, "Recibimos la tabla pibe");
		mandar_pcb_a_CPU(proceso);
		sem_post(&sem_recibir);
		break;
	}
}

void eliminarSegmento(int id, t_pcb* proceso) {
	t_paquete *paquete = crear_paquete(DELETE_SEGMENT);

	agregar_valor_estatico(paquete, &(proceso->pid));
	agregar_valor_estatico(paquete, &(id));

	//enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);
}

void recibirTablaActualizada(t_pcb* proceso) {
	proceso->tablaSegmentos = recibirTablaSegmentos(conexionMemoria);
	log_info(logger, "Recibimos la tabla actualizada dsp de eliminar el segmentoide");

}

