#include "utilsKernel.h"

#include <comunicacion.h>

int processID=0;
uint32_t BLOQUEADO = 0;
extern int cantidad_recursos;
uint32_t CONTINUAR_EJECUTANDO = 1;
extern t_config* config;
extern t_log* logger;
extern t_socket conexionCPU;
extern t_socket conexionMemoria;

extern sem_t sem_recibir_cpu, sem_proceso_fs_rw;

extern t_list* procesosReady;
extern pthread_mutex_t mutex_procesos_ready;

extern t_list* listaProcesosGlobal;


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
    pcb->archivosAbiertos = list_create();
    pcb->instanciasPorRecurso = malloc(sizeof(int)*cantidad_recursos);

    for(int i=0;i<cantidad_recursos;i++){
    	(pcb->instanciasPorRecurso)[i] = 0;
    }

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
		//liberarTablaSegmentos(pcb->tablaSegmentos);
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
	t_segmento* segmento;
	switch(cod) {
	case CREACION_EXITOSA:
		/*segmento->id = id;
		//recv(conexionMemoria, &segmento->base, sizeof(int) , MSG_WAITALL);
		segmento->limite = segmento->base + tamanio;
		//crearSegmento(proceso->tablaSegmentos, segmento) agrega el segmento en la tabla
		free(segmento);*/
		segmento = recibirSegmento(conexionMemoria);
		list_add(proceso->tablaSegmentos, segmento);
		mandar_pcb_a_CPU(proceso);
		sem_post(&sem_recibir_cpu);
		break;
	case OUT_OF_MEMORY:
		/*avisar_fin_a_consola(proceso->socket_consola);
		sem_post(&sem_grado_multiprogramacion);*/
		log_info(logger, "Finaliza el proceso %d - Motivo: OUT_OF_MEMORY", proceso->pid);
		log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", proceso->pid);
		finalizar_proceso("OUT_OF_MEMORY");
		//liberar_pcb(proceso);
		break;
	case LIMITE_SEGMENTOS_SUPERADO: //todo: mejorar esto, con solo un case de error de creacion reicibiendo el motivo x socket
		/*avisar_fin_a_consola(proceso->socket_consola);
		sem_post(&sem_grado_multiprogramacion);*/
		finalizar_proceso("LIMITE_SEGMENTOS_SUPERADO");
		log_info(logger, "Finaliza el proceso %d - Motivo: LIMITE_SEGMENTOS_SUPERADO", proceso->pid);
		log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", proceso->pid);
		//liberar_pcb(proceso);
		break;
	case PEDIDO_COMPACTAR:
		log_info(logger,"Compactación: Esperando Fin de Operaciones de FS");
		sem_wait(&sem_proceso_fs_rw);
		log_info(logger,"Compactación: Se solicitó compactación");
		int codOp = COMPACTAR;
		send(conexionMemoria, &codOp, sizeof(int), 0);

		actualizarTablasDeSegmentos(conexionMemoria, proceso);
		log_info(logger, "Se finalizó el proceso de compactación");

		//Tenemos que mandar otra vez el pedido a memoria
		solicitarCrearSegmento(id,tamanio, proceso); //mandamos a memoria

		// recibimos el segmento
		int codigo_op = recibir_operacion(conexionMemoria);
		if(codigo_op == CREACION_EXITOSA){
			segmento = recibirSegmento(conexionMemoria);
			list_add(proceso->tablaSegmentos, segmento);

			sem_post(&sem_proceso_fs_rw);
			mandar_pcb_a_CPU(proceso);
			sem_post(&sem_recibir_cpu);
		}else{
			log_error(logger, "El cod_op:%d que me mandó Memoria despues de compactar es invalido", cod);
			finalizar_proceso("ERROR AL COMPACTAR");
		}
		break;
	default:
		log_error(logger, "El cod_op:%d que me mandó Memoria es invalido", cod);
		finalizar_proceso("ERROR AL CREAR_SEG");
	}
}

t_pcb* obtenerProcesoPorPID(int pid, t_list* procesos){
	int cantidad = list_size(procesos);
	t_pcb* proceso;
	for(int i=0;i<cantidad;i++){
		proceso = list_get(procesos,i);
		if(proceso->pid == pid){
			printf("\n==El proceso %d esta en el indice %d==\n", pid, i);
			break;
		}
	}
	return proceso;
}

void obtenerProcesosReady(t_list* procesos){
	pthread_mutex_lock(&mutex_procesos_ready);
	int cantidad = list_size(procesosReady);
	pthread_mutex_unlock(&mutex_procesos_ready);
	printf("\nCantidad de procesos en ready: %d\n", cantidad);
	for(int i=0;i<cantidad;i++){
		pthread_mutex_lock(&mutex_procesos_ready);
		t_pcb* proceso = list_get(procesosReady,i);
		pthread_mutex_unlock(&mutex_procesos_ready);
		list_add(procesos, proceso);
	}
}


void actualizarTablaProceso(char* pidString, tabla_segmentos tabla, t_list* procesos){
	int pid = atoi(pidString);
	//printf("\nPidString %s\n", pidString);

	t_pcb* proceso = obtenerProcesoPorPID(pid, procesos);
	liberarTablaSegmentos(proceso->tablaSegmentos);
	proceso->tablaSegmentos = list_duplicate(tabla);
	printf("Pid proceso %d\n", proceso->pid);
	mostrarListaSegmentos(proceso->tablaSegmentos,logger);


}

void actualizarTablasDeSegmentos(int conexionMemoria, t_pcb* proceso){
	void* buffer;
	int size;
	int tamanio_diccionario,tamanio_tabla, tamanio_pid;
	int desplazamiento = 0;
	char* pid;

	int cod = recibir_operacion(conexionMemoria);
	switch(cod){
	case COMPACTAR:
		log_debug(logger, "Voy a actualizar tablas");
		/*t_list* procesos = obtenerTodosProcesosBloqueados();
		obtenerProcesosReady(procesos);
		agregarProcesosDeIO(procesos);
		list_add(procesos, proceso);
		char* lista = lista_procesos_string(procesos);*/
		char* lista = lista_procesos_string(listaProcesosGlobal);
		printf("Lista :[%s]\n", lista);


		buffer = recibir_buffer(&size, conexionMemoria);
		memcpy(&(tamanio_diccionario), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		log_debug(logger, "Cantidad procesos: %d", tamanio_diccionario);
		for(int i=0; i<tamanio_diccionario;i++){
			t_list* tabla = list_create();
			memcpy(&(tamanio_pid), buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			pid = malloc(tamanio_pid);
			memcpy(pid, buffer + desplazamiento,tamanio_pid);
			desplazamiento+=tamanio_pid;
			memcpy(&(tamanio_tabla), buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);

			for(int i=0; i<tamanio_tabla;i++){
				t_segmento* segmento = malloc(sizeof(t_segmento));
				memcpy(&(segmento->id), buffer + desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				memcpy(&(segmento->base), buffer + desplazamiento, sizeof(u_int32_t));
				desplazamiento+=sizeof(u_int32_t);
				memcpy(&(segmento->limite), buffer + desplazamiento, sizeof(u_int32_t));
				desplazamiento+=sizeof(u_int32_t);
				list_add(tabla, segmento);
			}

			actualizarTablaProceso(pid,tabla,listaProcesosGlobal);
			list_destroy(tabla);
			free(pid);
		}
		free(buffer);
		break;
	}

}

void solicitarEliminarSegmento(int id, t_pcb* proceso) {
	t_paquete *paquete = crear_paquete(DELETE_SEGMENT_OP);

	agregar_valor_estatico(paquete, &(proceso->pid));
	agregar_valor_estatico(paquete, &(id));

	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);
}

void recibirEliminarsegmento(t_pcb* proceso){
	int cod = recibir_operacion(conexionMemoria);
	if (cod == TABLA_SEGMENTOS){
		liberarTablaSegmentos(proceso->tablaSegmentos);
		proceso->tablaSegmentos = recibirTablaSegmentos(conexionMemoria);
		log_debug(logger, "Tabla de segmentos actualizada - Se eliminó el segmento");
	}
	mandar_pcb_a_CPU(proceso);
	sem_post(&sem_recibir_cpu);

}
