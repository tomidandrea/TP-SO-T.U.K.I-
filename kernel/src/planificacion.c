#include <planificacion.h>

extern t_config* config;
extern t_log* logger;

extern t_list* procesosExecute;
extern t_list* procesosReady;
extern t_list* esperaDeIO;
extern t_list* archivosAbiertosGlobal;
extern t_socket conexionFileSystem;

extern t_list* listaProcesosGlobal;
extern bool seguir_ejecucion;

uint32_t INICIO = 1;

sem_t sem_new_a_ready, sem_ready, sem_grado_multiprogramacion, sem_recibir_cpu, sem_recibir_fs, sem_execute, sem_proceso_fs_rw;
pthread_mutex_t mutex_procesos_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_procesos_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_procesos_execute = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_espera_FS = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_procesos_io = PTHREAD_MUTEX_INITIALIZER;
extern sem_t sem_finalizar;

void planificar(){
	char* algoritmo = malloc(16);
	t_pcb* proceso;
	algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");

	double alfa;
	alfa = config_get_double_value(config,"HRRN_ALFA");

	while (seguir_ejecucion){

		sem_wait(&sem_execute);
		sem_wait(&sem_ready);
		if(seguir_ejecucion==0){
			log_debug(logger, "Salgo del hilo plani");
			sem_post(&sem_finalizar);
		}else{
			log_info(logger, "Empezando a planificar");
			pthread_mutex_lock(&mutex_procesos_ready);
			char* mensaje = lista_procesos_string(procesosReady);
			pthread_mutex_unlock(&mutex_procesos_ready);
			log_info(logger, "Cola Ready %s: [%s]", algoritmo, mensaje);
			free(mensaje);
			//mostrarListaProcesos(procesosReady);
			//sem_wait(&sem_ready); // Agrego otro semaforo para que no empiece a planificar por ahora

			if(strcmp(algoritmo,"FIFO") == 0){
				proceso = planificarFIFO();
				log_info(logger, "Termine de planificar FIFO");
			}else if(strcmp(algoritmo,"HRRN") == 0){
				proceso = planificarHRRN(alfa);
				log_info(logger, "Termine de planificar HRRN");
			}else{
				log_error(logger, "Nombre del algoritmo invalido");
			}

			mandar_pcb_a_CPU(proceso);
			log_info(logger, "Proceso %d enviado a cpu\n", proceso->pid);
			sem_post(&sem_recibir_cpu);
		}


	}
	log_debug(logger, "Sali while de planificar");
	free(algoritmo);
}

void recibirDeCPU() {

	while(seguir_ejecucion) {
	sem_wait(&sem_recibir_cpu);
	if(seguir_ejecucion==0){
		log_debug(logger, "Salgo del hilo cpu");
		sem_post(&sem_finalizar);
	}else{
		t_pcb* proceso = list_get(procesosExecute, 0);
		t_contexto* contexto = actualizar_pcb(proceso);
		t_archivo* archivoProceso;
		char* recurso;
		int id, cant_bytes;
		u_int32_t direc_fisica;

			switch(contexto->motivo){
				case EXT:
					finalizar_proceso("SUCCESS");
					break;
				case SEG_FAULT:
					finalizar_proceso("SEG_FAULT");
					break;
				case YIELD:
					log_info(logger, "Hubo un YIELD del proceso %d\n", proceso->pid);

					proceso = removerDeExecute();
					pasarAReady(proceso);
					log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: READY", proceso->pid);

					break;
				case IO:
					log_info(logger, "Hubo un IO de PID:%d\n", contexto->pid);
					int tiempo = atoi(contexto->parametros[0]);
					proceso = removerDeExecute();
					io_contexto* ioContexto = inicializarIoContexto(proceso, tiempo);
					ejecutarIO(ioContexto);
					pthread_mutex_lock(&mutex_procesos_io);
					list_add(esperaDeIO, proceso);
					pthread_mutex_unlock(&mutex_procesos_io);
					break;
				case WAIT:
					//proceso = removerDeExecute();
					log_info(logger, "Llego un WAIT pibe\n");
					recurso = contexto->parametros[0];
					if(verificarRecursos(recurso)){
						wait(proceso, recurso);
					} else{
						finalizar_proceso("INVALID_RESOURCE");
					}
					break;
				case SIGNAL:
					//INICIO = 1; //para que no haga el calculo HRRN y agarre el primero de ready
					//proceso = removerDeExecute();
					log_info(logger, "Llego un SIGNAL pibe\n");
					recurso = contexto->parametros[0];
					if(verificarRecursos(recurso)){
						ejecutarSignal(proceso, recurso);
					} else{
						finalizar_proceso("INVALID_RESOURCE");
					}
					break;
				case CREATE_SEGMENT:
					log_info(logger, "Llego un CREATE_SEGMENT pibe\n");
					id = atoi(contexto->parametros[0]);
					int tamanio = atoi(contexto->parametros[1]);
					solicitarCrearSegmento(id,tamanio, proceso); //mandamos a memoria
					log_info(logger, "PID: %d - Crear Segmento - Id: %d - Tamaño: %d", proceso->pid, id, tamanio);
					recibirCrearSegmento(id, tamanio, proceso);
					break;
				case DELETE_SEGMENT:
					log_info(logger, "Llego un DELETE_SEGMENT pibe\n");
					id = atoi(contexto->parametros[0]);
					solicitarEliminarSegmento(id, proceso);
					log_info(logger, "PID: %d - Eliminar Segmento - Id: %d", proceso->pid, id);
					recibirEliminarsegmento(proceso);
					break;
				case F_OPEN:
					log_info(logger, "Hubo un F_OPEN de PID:%d\n", contexto->pid);
					t_archivo* archivo = inicializarArchivo(contexto->parametros[0]);
					log_info(logger, "PID: %d - Abrir Archivo: %s", contexto->pid, archivo->nombre);
					//Si esta en la tabla de archivos abiertos lo agrego a la tabla de archivos del proceso con puntero 0 y bloqueo al proceso
					if(estaAbiertoElArchivo(archivo->nombre)) {
						log_debug(logger, "El archivo %s ya fue abierto por un proceso", archivo->nombre);
						list_add(proceso->archivosAbiertos, archivo);
						t_archivo_global* archivoAbiertoGlobal = archivoGlobalQueSeLlama(archivo->nombre);
						t_pcb* proceso = removerDeExecute();
						bloquearEnColaDeArchivo(archivoAbiertoGlobal, proceso);

					} else { //sino le mando al FS para saber si hay que crearlo
						abrirArchivoEnFS(archivo->nombre);
						list_add(proceso->archivosAbiertos, archivo);
						t_archivo_global* archivoGlobal = inicializarArchivoGlobal(archivo->nombre);
						list_add(archivosAbiertosGlobal, archivoGlobal);
						mandar_pcb_a_CPU(proceso);
						sem_post(&sem_recibir_cpu);

					}
					break;
				case F_CLOSE:
					log_info(logger, "Hubo un F_CLOSE de PID:%d\n", contexto->pid);
					t_archivo_global* archivoAbiertoGlobal = archivoGlobalQueSeLlama(contexto->parametros[0]);
					archivoProceso = archivoQueSeLlama(contexto->parametros[0], proceso->archivosAbiertos);
					//todo: hacer if su archivo proceso es NULL?
					log_info(logger, "PID: %d - Cerrar Archivo: %s", contexto->pid, archivoProceso->nombre);
					// Si no hay nadie en la cola del archivo lo saco de la tabla global y del proceso
					if(queue_is_empty(archivoAbiertoGlobal->cola)) {
						list_remove_element(archivosAbiertosGlobal, archivoAbiertoGlobal);
						liberarArchivoGlobal(archivoAbiertoGlobal);
						list_remove_element(proceso->archivosAbiertos, archivoProceso);
						liberarArchivo(archivoProceso);

					} else { //sino desbloqueo al primer proceso de la cola
						desbloquearDeColaDeArchivo(archivoAbiertoGlobal);
					}
					mandar_pcb_a_CPU(proceso);
					sem_post(&sem_recibir_cpu);

					break;
				case F_SEEK:
					log_info(logger, "Hubo un F_SEEK de PID:%d\n", contexto->pid);
					int nuevoPuntero = atoi(contexto->parametros[1]);
					archivoProceso = archivoQueSeLlama(contexto->parametros[0], proceso->archivosAbiertos);
					//todo: hacer if su archivo proceso es NULL?
					log_debug(logger, "Valor del puntero antes de ejecutar fseek: %d", archivoProceso->puntero);
					//Si saca de la lista al archivo pasado por parametro, actualizo el puntero y lo meto de nuevo
					actualizar_puntero(proceso,archivoProceso, nuevoPuntero);
					mandar_pcb_a_CPU(proceso);
					sem_post(&sem_recibir_cpu);

					break;
				case F_TRUNCATE:
					log_info(logger, "Hubo un F_TRUNCATE de PID:%d\n", contexto->pid);
					char* nombreArchivo = copiar(contexto->parametros[0]);
					int tamanioATruncar = atoi(contexto->parametros[1]);
					truncar_archivo(nombreArchivo, tamanioATruncar);
					log_info(logger, "PID: %d - Archivo: %s - Tamaño: %d", contexto->pid, nombreArchivo, tamanioATruncar);
					free(nombreArchivo);
					break;
				case F_READ:
					log_info(logger, "Hubo un F_READ de PID:%d\n", contexto->pid);
					archivoProceso = archivoQueSeLlama(contexto->parametros[0], proceso->archivosAbiertos);
					//todo: hacer if su archivo proceso es NULL?
					direc_fisica = contexto->direc_fisica;
					cant_bytes = atoi(contexto->parametros[2]);
					leer_archivo(contexto->pid, archivoProceso, direc_fisica, cant_bytes);
					//esto no se si hace o no, seria actualizar el puntero cada vez que escribe o lee: actualizar_puntero(proceso,archivoProceso, archivoProceso->puntero + cant_bytes);
					break;
				case F_WRITE:
					log_info(logger, "Hubo un F_WRITE de PID:%d\n", contexto->pid);
					archivoProceso = archivoQueSeLlama(contexto->parametros[0], proceso->archivosAbiertos);
					//todo: hacer if su archivo proceso es NULL?
					direc_fisica = contexto->direc_fisica;
					cant_bytes = atoi(contexto->parametros[2]);
					escribir_archivo(contexto->pid, archivoProceso, direc_fisica, cant_bytes);
					//esto no se si hace o no: actualizar_puntero(proceso,archivoProceso, archivoProceso->puntero + cant_bytes);
					break;
				default:
					log_debug(logger, "No se implemento la instruccion");
					finalizar_proceso("INSTRUCCION INVALIDA");
					break;
			}
			liberar_contexto(contexto);
	}

	}
	log_debug(logger, "Sali while de recibirCPU");
}

void recibirDeFS() {

	while(seguir_ejecucion) {
	sem_wait(&sem_recibir_fs);

	if(seguir_ejecucion==0){
		log_debug(logger, "Salgo del hilo fs");
		sem_post(&sem_finalizar);
	}else{
		if(conexionFileSystem != -1){

			int cod_op = recibir_operacion(conexionFileSystem);

			switch(cod_op){
				case MENSAJE:
					char*mensaje = recibir_mensaje(conexionFileSystem, logger);
					if(strcmp(mensaje, "OP_OK") == 0) {
						desbloquearDeEsperaDeFS();
					} else if(strcmp(mensaje, "OP_READ") == 0 || strcmp(mensaje, "OP_WRITE") == 0){
						desbloquearDeEsperaDeFS();
						sem_post(&sem_proceso_fs_rw);
					} else
						log_error(logger, "No se realizo correctamente la operacion en FS");
					//free(mensaje);
				break;

				default:
					log_debug(logger, "No me llego un mensaje");
			}
		} else {
			log_error(logger, "Se rompio la conexion con File System");
			break;
		}
	}
	}
	log_debug(logger, "Sali while de escucharFS");
}

void finalizar_proceso(char* motivo) {
	t_pcb* proceso = removerDeExecute();
	log_info(logger, "=== Salimos como unos campeones!!!!, PID:%d finalizó ===\n", proceso->pid);
	log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", proceso->pid);
	log_info(logger, "Finaliza el proceso %d - Motivo: %s", proceso->pid, motivo);
	liberar_recursos(proceso);
	avisar_fin_a_memoria(proceso->pid);
	avisar_fin_a_consola(proceso->socket_consola);
	removerProcesoPorPID(listaProcesosGlobal, proceso->pid);
	liberar_pcb(proceso);
	log_debug(logger, "Lista procesosReady:%d", list_size(procesosReady));
	log_debug(logger, "POST grado multi");
	//sem_post(&sem_grado_multiprogramacion);
}
void agregarReady(){
	
	while (seguir_ejecucion)
	{
		sem_wait(&sem_new_a_ready);
		sem_wait(&sem_grado_multiprogramacion);
		if(seguir_ejecucion==0){
			log_debug(logger, "Salgo del hilo ready");
			sem_post(&sem_finalizar);
		}else{
			log_info(logger, "Permite agregar proceso a ready por grado de multiprogramacion\n");
			pasarNewAReady();
		}
	}
	log_debug(logger, "Sali while de agregarReady");
}

t_pcb* planificarFIFO(){
	t_pcb* proceso = removerPrimeroDeReady();
	pasarAExecute(proceso);
	log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);
	return proceso;
}

int procesoConMayorRatio(int cant){
	double ratio = 0;
	t_pcb* proceso;
	int max;
	for(int i=0;i<cant;i++){
		proceso = list_get(procesosReady,i);
		if(ratio < proceso->ratio){
			ratio = proceso->ratio;
			max = i;
		}
	}
	return max;
}

t_pcb* planificarHRRN(double alfa){

	t_pcb* proceso = inicializar_pcb();

	if(INICIO){
		proceso = removerPrimeroDeReady();
		proceso->tiempoEnReady= pararTiempo(proceso->tiempoEnReady);
		//pararTiempoReady(proceso);
		INICIO = 0;
	}
	else {
		int64_t tiempoEnReady;
		double est;
		double estActual;
		int cant = list_size(procesosReady);

		for(int i = 0;i<cant;i++) {
			proceso = list_get(procesosReady,i);
			tiempoEnReady = temporal_gettime(proceso->tiempoEnReady);
			log_debug(logger, "Proceso %d tiempoEnReady : %ld", proceso->pid ,tiempoEnReady);
			est = proceso->estimadoAnterior;

			// calculo la rafaga actual
			if(proceso->pc == 0){
				estActual = proceso->estimadoAnterior;
			}
			else{
				int64_t realEjec;
				realEjec = temporal_gettime(proceso->tiempoCPU);
				proceso->tiempoCPU = pararTiempo(proceso->tiempoCPU);
				//pararTiempoCPU(proceso);

				estActual = alfa*realEjec + (1-alfa)*est;

				proceso->estimadoAnterior = estActual;
				//temporal_resume(proceso->tiempoCPU);
				//printf("realEjec %ld \n", realEjec);
				//printf("proceso->tiempoCPU %ld \n", temporal_gettime(proceso->tiempoCPU));

			}

			// calculo el ratio
			proceso->ratio = (tiempoEnReady + estActual)/estActual;
			log_info(logger,"RATIO: %f - proceso: %d", proceso->ratio, proceso->pid);

		}
		// aca se puede usar la funcion de las commons list_get_maximum
		proceso = list_remove(procesosReady, procesoConMayorRatio(cant));
		proceso->tiempoEnReady = pararTiempo(proceso->tiempoEnReady);
		//pararTiempoReady(proceso);
	}

	pasarAExecute(proceso);

	log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);

	return proceso;

}


