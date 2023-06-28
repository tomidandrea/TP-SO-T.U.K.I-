#include <manejoArchivos.h>

extern t_log* logger;
extern t_list* archivosAbiertosGlobal;
extern t_socket conexionFileSystem;
extern t_list* esperaDeFS;
extern sem_t sem_recibir_fs;

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

	sem_post(&sem_recibir_fs);

}

void truncar_archivo(char* nombre, int tamanio) {
	t_paquete *paquete = crear_paquete(F_TRUNCATE);
	agregar_a_paquete(paquete, nombre, strlen(nombre) + 1);
	agregar_valor_estatico(paquete, &tamanio);

	enviar_paquete(paquete,conexionFileSystem);
	eliminar_paquete(paquete);

	t_pcb* proceso = removerDeExecute();
	bloquearPorFS(proceso, "F_TRUNCATE");

	sem_post(&sem_recibir_fs);
}

void bloquearPorFS(t_pcb* proceso, char* motivo) {

	list_add(esperaDeFS, proceso);
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

