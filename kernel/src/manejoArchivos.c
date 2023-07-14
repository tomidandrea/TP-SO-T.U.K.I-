#include <manejoArchivos.h>

extern t_log* logger;
extern t_list* archivosAbiertosGlobal;
extern t_socket conexionFileSystem;
extern t_list* esperaDeFS;
extern sem_t sem_recibir_fs;
extern sem_t sem_proceso_fs_rw;
extern pthread_mutex_t mutex_espera_FS;

t_archivo_global* inicializarArchivoGlobal(char * nombre) {
	t_archivo_global* archivoAbiertoGlobal = malloc(sizeof(t_archivo_global));
	archivoAbiertoGlobal->nombre = copiar(nombre);
	archivoAbiertoGlobal->cola = queue_create();

	return archivoAbiertoGlobal;
}

t_archivo* inicializarArchivo(char * nombre) {
	t_archivo* archivo = malloc(sizeof(t_archivo));
	archivo->nombre = copiar(nombre);
	archivo->puntero = 0;
	return archivo;
}
int estaAbiertoElArchivo(char * nombre) {
	int cantArchivos = list_size(archivosAbiertosGlobal);
	for(int i=0; i<cantArchivos; i++) {
		t_archivo_global* archivo = list_get(archivosAbiertosGlobal,i);
		if(strcmp(archivo->nombre, nombre) == 0) {
			return 1;
		}
	}
	return 0;
}

t_archivo_global* archivoGlobalQueSeLlama(char* nombre) {
	int cantArchivos = list_size(archivosAbiertosGlobal);
	for(int i=0; i<cantArchivos; i++) {
		t_archivo_global* archivo = list_get(archivosAbiertosGlobal,i);
		if(strcmp(archivo->nombre, nombre) == 0) {
			return archivo;
		}
	}
	return NULL;
}

t_archivo* archivoQueSeLlama(char* nombre, tabla_archivos archivosAbiertos) {
	int cantArchivos = list_size(archivosAbiertosGlobal);
		for(int i=0; i<cantArchivos; i++) {
			t_archivo* archivo = list_get(archivosAbiertos,i);
			if(strcmp(archivo->nombre, nombre) == 0) {
				return archivo;
			}
		}
	return NULL;
}

void liberarArchivoGlobal(t_archivo_global* archivo) {
	free(archivo->nombre);
	queue_destroy(archivo->cola);
	free(archivo);
}

void liberarArchivo(t_archivo* archivo) {
	free(archivo->nombre);
	free(archivo);
}

void abrirArchivoEnFS(char* nombre) {

	t_paquete *paquete = crear_paquete(F_OPEN);
	agregar_a_paquete(paquete, nombre, strlen(nombre) + 1);

	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);

	int cod_op;
	if(recv(conexionFileSystem, &cod_op, sizeof(int), MSG_WAITALL) > 0){
			 char* mensaje=recibir_mensaje(conexionFileSystem, logger);
				log_debug(logger,"Me llego de FS el resultado: %s",mensaje);
				if(strcmp(mensaje, "OP_OK") == 0) {
					log_debug(logger,"El archivo %s ya estaba creado en el FS!!", nombre);
				} else
					if(strcmp(mensaje, "OP_ERROR") == 0) {
						log_debug(logger, "Enviando creacion del archivo %s al FS", nombre);
						crearArchivoEnFS(nombre);
					} else log_error(logger,"Me llego cualquier cosa");

			} else {
				log_error(logger,"No me llego el resultado de la operacion en FS");
			}
}

void crearArchivoEnFS(char* nombre) {
	t_paquete *paquete = crear_paquete(CREAR_ARCHIVO);
	agregar_a_paquete(paquete, nombre, strlen(nombre) + 1);

	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);

	int cod_op;
	if(recv(conexionFileSystem, &cod_op, sizeof(int), MSG_WAITALL) > 0){
		char* mensaje=recibir_mensaje(conexionFileSystem, logger);
		log_debug(logger,"Me llego de FS el resultado: %s",mensaje);
		if(strcmp(mensaje, "OP_OK") == 0) {
			log_debug(logger,"El archivo %s se creó exitosamente!!", nombre);
		} else log_error(logger,"Me llego cualquier cosa");
	} else {
		log_error(logger,"No me llego el resultado de FS");
	}
}

void truncar_archivo(char* nombreArchivo, int tamanio) {
	t_paquete *paquete = crear_paquete(F_TRUNCATE);
	agregar_a_paquete(paquete, nombreArchivo, strlen(nombreArchivo) + 1);
	agregar_valor_estatico(paquete, &tamanio);

	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);

	t_pcb* proceso = removerDeExecute();

	bloquearPorFS(proceso, "F_TRUNCATE");
	sem_post(&sem_recibir_fs);
}

void leer_archivo(t_archivo* archivo, u_int32_t direc_fisica, int cant_bytes) {

	t_paquete *paquete = crear_paquete(F_READ);
	agregar_a_paquete(paquete, archivo->nombre, strlen(archivo->nombre) + 1);
	agregar_valor_estatico(paquete, &(archivo->puntero));
	agregar_valor_uint(paquete, &direc_fisica);
	agregar_valor_estatico(paquete, &cant_bytes);


	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);

	t_pcb* proceso = removerDeExecute();
	log_info(logger,"PID: %d - Leer Archivo: %s - Puntero %d - Dirección Memoria %u - Tamaño %d", proceso->pid, archivo->nombre, archivo->puntero, direc_fisica, cant_bytes);
	bloquearPorFS(proceso, "F_READ");
	sem_wait(&sem_proceso_fs_rw);
	sem_post(&sem_recibir_fs);
}

void escribir_archivo(t_archivo* archivo, u_int32_t direc_fisica, int cant_bytes) {
	t_paquete *paquete = crear_paquete(F_WRITE);
	agregar_a_paquete(paquete, archivo->nombre, strlen(archivo->nombre) + 1);
	agregar_valor_estatico(paquete, &(archivo->puntero));
	agregar_valor_uint(paquete, &direc_fisica);
	agregar_valor_estatico(paquete, &cant_bytes);

	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);

	t_pcb* proceso = removerDeExecute();
	log_info(logger,"PID: %d - Escribir Archivo: %s - Puntero %d - Dirección Memoria %u - Tamaño %d", proceso->pid, archivo->nombre, archivo->puntero, direc_fisica, cant_bytes);
	bloquearPorFS(proceso, "F_WRITE");
	sem_wait(&sem_proceso_fs_rw);
	sem_post(&sem_recibir_fs);
}

void actualizar_puntero(t_pcb* proceso, t_archivo* archivoProceso, int nuevoPuntero) {
	if(list_remove_element(proceso->archivosAbiertos, archivoProceso)) {
		archivoProceso->puntero = nuevoPuntero;
		list_add(proceso->archivosAbiertos, archivoProceso);
		log_info(logger, "PID: %d - Actualizar puntero Archivo: %s - Puntero %d", proceso->pid, archivoProceso->nombre, archivoProceso->puntero);
	} else {
		log_error(logger, "No se pudo actualizar el puntero del archivo %s del proceso %d", archivoProceso->nombre, proceso->pid);
	}
}

void bloquearPorFS(t_pcb* proceso, char* motivo) {

	pthread_mutex_lock(&mutex_espera_FS);
	list_add(esperaDeFS, proceso);
	pthread_mutex_unlock(&mutex_espera_FS);
	log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: BLOCKED", proceso->pid);
	log_info(logger, "PID: %d - Bloqueado por: %s", proceso->pid, motivo);
}

void bloquearEnColaDeArchivo(t_archivo_global* archivo, t_pcb* proceso) {
	queue_push(archivo->cola, proceso);
	log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: BLOCKED", proceso->pid);
	log_info(logger, "PID: %d - Bloqueado por: %s", proceso->pid, archivo->nombre);

}

void desbloquearDeColaDeArchivo(t_archivo_global* archivo) {
	t_pcb* proceso = queue_peek(archivo->cola);
	pasarAReady(proceso);
	log_info(logger, "Se desbloqueo el proceso %d de la cola del archivo %s",proceso->pid, archivo->nombre);
	log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", proceso->pid);
	queue_pop(archivo->cola);
}

void desbloquearDeEsperaDeFS() {
	pthread_mutex_lock(&mutex_espera_FS);
	t_pcb* proceso = list_remove(esperaDeFS, 0);
	pthread_mutex_unlock(&mutex_espera_FS);
	pasarAReady(proceso);
	log_debug(logger, "Se desbloqueo el proceso %d de la cola de espera de FS",proceso->pid);
    log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", proceso->pid);
}

