#include "comunicacionMemoria.h"

extern t_socket server_fd;
extern t_log* logger;
extern sem_t sem_cpu, sem_kernel;
extern t_dictionary* diccionarioTablas;
extern t_segmento* segmento0;
extern int cantidadMaxSegmentos;
extern op_code estadoCreacion;

uint32_t RESULT_ERROR = 1;

// Hilo que atiende peticiones de kernel

void escucharKernel(){
	log_debug(logger, "Entro hilo para escuchar kernel");
	sem_wait(&sem_cpu); //esperar que se conecte primero cpu
	t_socket socket_kernel = esperar_cliente(server_fd, logger);
	char* pid;
	tabla_segmentos tablaSegmentos;
	t_pedido_segmento* pedido = malloc(sizeof(t_pedido_segmento));

	while(1){
		if(socket_kernel != -1){
			int cod_op = recibir_operacion(socket_kernel);
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
				if(cantidadSegmentos < cantidadMaxSegmentos) {
					crearSegmento(pedido);

					switch (estadoCreacion) {
					case CREACION_EXITOSA:
						enviarSegmentoCreado(socket_kernel, tablaSegmentos);
						break;
					case OUT_OF_MEMORY:
						log_error(logger,"Límite de segmentos máximos alcanzado - Limite: %d", cantidadMaxSegmentos);
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
						break;
					}
				}
				else{
					estadoCreacion = LIMITE_SEGMENTOS_SUPERADO;
					send(socket_kernel, &estadoCreacion, sizeof(op_code), 0);
				}
				break;
			case DELETE_SEGMENT_OP:
				pedido = recibirPedidoDeleteSegment(socket_kernel);

				log_debug(logger, "Delete segment: PID: %d - Id segmento: %d", pedido->pid, pedido->id_segmento);

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
				log_error(logger,"Se cerró la conexión");
				exit(1);
			}
		}else{
			log_error(logger,"Se cerró la conexión");
		}
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
			// cosas q pide cpu xd
		}
	}
}

void escucharFS(){
	log_debug(logger, "Entro hilo para escuchar FS");
	t_socket socket_file_system;
	//t_socket socket_file_system = esperar_cliente(server_fd, logger);
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
