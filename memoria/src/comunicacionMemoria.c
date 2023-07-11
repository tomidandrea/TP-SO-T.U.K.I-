#include "comunicacionMemoria.h"

extern t_socket server_fd;
extern t_log* logger;
extern t_config* config;
extern sem_t sem_cpu, sem_kernel, ejecutando;
extern t_dictionary* diccionarioTablas;
extern t_segmento* segmento0;
extern void* espacioMemoria;
extern int retardo_memoria;


extern int cantidadMaxSegmentos;
extern op_code estadoCreacion;
uint32_t RESULT_ERROR = 1;
uint32_t RESULT_OK = 0;

// Hilo que atiende peticiones de kernel

void escucharKernel(){
	log_debug(logger, "Entro hilo para escuchar kernel");
	sem_wait(&sem_cpu); //esperar que se conecte primero cpu
	t_socket socket_kernel = esperar_cliente(server_fd, logger);
	char* pid;
	//t_pedido_segmento* pedido = malloc(sizeof(t_pedido_segmento));
	t_pedido_segmento* pedido;

	while(1){
	tabla_segmentos tablaSegmentos;
		if(socket_kernel != -1){
			int cod_op = recibir_operacion(socket_kernel);
			sem_wait(&ejecutando);
			switch (cod_op) {
			case TABLA_SEGMENTOS:
				tablaSegmentos = list_create();
				list_add(tablaSegmentos, segmento0);

				pid = recibirPID(socket_kernel);
				dictionary_put(diccionarioTablas, pid, tablaSegmentos);

				log_info(logger, "Creación de Proceso PID: %s", pid);
				enviarSegmentosKernel(socket_kernel, tablaSegmentos);

				break;
			case CREATE_SEGMENT_OP:
				pedido = recibirPedidoSegmento(socket_kernel);

				pid = string_itoa(pedido->pid);
				tablaSegmentos = dictionary_get(diccionarioTablas, pid);

				int cantidadSegmentos = list_size(tablaSegmentos);
				op_code estadoCreacion;
				if(cantidadSegmentos < cantidadMaxSegmentos) {
					estadoCreacion = crearSegmento(pedido);

					switch (estadoCreacion) {
					case CREACION_EXITOSA:
						enviarSegmentoCreado(socket_kernel, tablaSegmentos);
						break;
					case OUT_OF_MEMORY:
						log_error(logger,"No hay espacio suficiente en memoria para crear el segmento");
						send(socket_kernel, &estadoCreacion, sizeof(op_code), 0);
						break;
					case PEDIDO_COMPACTAR:
						send(socket_kernel, &estadoCreacion, sizeof(op_code), 0);
						int cod_op = recibir_operacion(socket_kernel);
						if (cod_op == COMPACTAR){
							compactar(pedido);
							enviarDiccionarioTablas(socket_kernel);
						}
						break;
					default:
						log_error(logger,"Error en obtener estado memoria");
						liberar_memoria();
						break;
					}
				}
				else{
					estadoCreacion = LIMITE_SEGMENTOS_SUPERADO;
					log_error(logger,"Límite de segmentos máximos alcanzado - Limite: %d", cantidadMaxSegmentos);
					liberar_memoria();
					send(socket_kernel, &estadoCreacion, sizeof(op_code), 0);
				}
				break;
			case DELETE_SEGMENT_OP:
				pedido = recibirPedidoDeleteSegment(socket_kernel);
				log_debug(logger, "Delete segment: PID: %d - Id segmento: %d", pedido->pid, pedido->id_segmento);

				pid = string_itoa(pedido->pid);
				tablaSegmentos = dictionary_get(diccionarioTablas, pid);

				eliminarSegmento(pedido);
				enviarSegmentosKernel(socket_kernel, tablaSegmentos);
				break;
			case FIN_PROCESO:
				pid = recibirPID(socket_kernel);
				liberarEstructurasProceso(pid);
				log_info(logger, "Eliminación de Proceso PID: %s", pid);
				break;
			default:
				send(socket_kernel, (void *)(intptr_t)RESULT_ERROR, sizeof(uint32_t), (intptr_t)NULL);
				log_error(logger,"Se cerró la conexión / Cod op invalido");
				liberar_memoria();
				exit(1);
			}
			//free(pedido);
		}else{
			liberar_memoria();
			log_error(logger,"Se cerró la conexión");
		}

		sem_post(&ejecutando);
	}
	free(pid);
	free(pedido);
}

void escucharCPU(){
	log_debug(logger, "Entro hilo para escuchar CPU");
	t_socket socket_cpu = esperar_cliente(server_fd, logger);
	sem_post(&sem_cpu);
	while(1){
		if(socket_cpu != -1){
			int cod_op = recibir_operacion(socket_cpu);
			sem_wait(&ejecutando);
			u_int32_t direc_fisica;
			int pid, tamanio, size;
			void* buffer;
			int desplazamiento = 0;

			switch (cod_op) {
			case LEER:
				//recibo los datos necesarios para realizar la lectura
				buffer = recibir_buffer(&size, socket_cpu);
				memcpy(&pid, buffer + desplazamiento, sizeof(int));
			    desplazamiento += sizeof(int);
				memcpy(&direc_fisica, buffer + desplazamiento, sizeof(u_int32_t));
				desplazamiento += sizeof(u_int32_t);
				memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
				log_debug(logger,"Case LEER, recibi pid %d, dir fisica %d, tamaño %d", pid, direc_fisica, tamanio);
				//devuelvo el valor leido en la direccion pedida
				char* valor_leido = leer(direc_fisica, tamanio, pid);
				//valor_leido[tamanio] = '\0';
				char* valor_a_enviar=malloc(tamanio+1);
				memcpy(valor_a_enviar, valor_leido, tamanio);
				valor_a_enviar[tamanio] = '\0';

				log_debug(logger,"Lei el valor %s de tamaño %d. Enviando a CPU...", valor_a_enviar, tamanio);
				//hago el retardo que pide el enuncuado por acceder al espacio de memoria
				usleep(retardo_memoria * 1000);

				enviar_mensaje(valor_a_enviar, socket_cpu);

				free(valor_leido);
				free(buffer);
				break;
			case ESCRIBIR:
				//recibo los datos para escritura
				buffer = recibir_buffer(&size, socket_cpu);
				memcpy(&pid, buffer + desplazamiento, sizeof(int));
				desplazamiento += sizeof(int);
				memcpy(&direc_fisica, buffer + desplazamiento, sizeof(u_int32_t));
				desplazamiento += sizeof(u_int32_t);
				memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
				desplazamiento += sizeof(int);
				log_debug(logger,"Case ESCRIBIR, recibi pid %d, dir fisica %d, tamaño %d", pid, direc_fisica, tamanio);

				char* valor = malloc(tamanio);
	     		memcpy(valor, buffer + desplazamiento, tamanio);
	     		//valor[tamanio] = '\0';
	     		char* para_mostrar = malloc(tamanio+1);
	     		memcpy(para_mostrar, buffer + desplazamiento, tamanio);
	     		para_mostrar[tamanio] = '\0';
	     		log_debug(logger,"Me llego una escritura del valor %s en la direccion %d ", para_mostrar, direc_fisica);
				free(para_mostrar);

	     		escribir(direc_fisica, tamanio, valor, pid);
				usleep(retardo_memoria * 1000);
				//le mando OK a cpu para que siga ejecutando
				//send(socket_cpu, &RESULT_OK, tamanio, 0);
				char* mensaje="OK";
				enviar_mensaje(mensaje, socket_cpu);
				free(valor);
				free(buffer);
			}
			// cosas q pide cpu xd
		}
		sem_post(&ejecutando);
	}
}


void escucharFS(){
	log_debug(logger, "Entro hilo para escuchar FS");
	t_socket socket_file_system = esperar_cliente(server_fd, logger);
	while(1){
		if(socket_file_system != -1){
			int cod_op = recibir_operacion(socket_file_system);
			// cosas q pide file system xd
		}
	}
}

void enviarDiccionarioTablas(t_socket socket_kernel){
	t_paquete* paquete = crear_paquete(COMPACTAR);
	int cantidadProcesos = dictionary_size(diccionarioTablas);
	agregar_valor_estatico(paquete,&cantidadProcesos);

	void serializarTablaSegmentos(char* pid, void* tablaSegmentos){
		t_segmento* segmento;
		int cantidad = list_size(tablaSegmentos);
		agregar_a_paquete(paquete, pid, strlen(pid)+1);
		agregar_valor_estatico(paquete,&cantidad);
		for (int i = 0; i<cantidad; i++){
			segmento = list_get(tablaSegmentos, i);
			agregar_valor_estatico(paquete,&(segmento->id));
			agregar_valor_uint(paquete,&(segmento->base));
			agregar_valor_uint(paquete,&(segmento->limite));
		}
	}

	dictionary_iterator(diccionarioTablas, serializarTablaSegmentos);
	log_debug(logger, "Diccionario serializado");
	enviar_paquete(paquete, socket_kernel);
	eliminar_paquete(paquete);
}
