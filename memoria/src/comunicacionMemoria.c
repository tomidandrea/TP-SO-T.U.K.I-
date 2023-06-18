#include "comunicacionMemoria.h"

extern t_socket server_fd;
extern t_log* logger;
extern t_config* config;
extern sem_t sem_cpu, sem_kernel;
extern t_dictionary* diccionarioTablas;
extern t_segmento* segmento0;
extern void* espacioMemoria;
extern int retardo_memoria;


extern int cantidadMaxSegmentos;
extern op_code estadoCreacion;
uint32_t RESULT_ERROR = 1;
uint32_t RESULT_OK = 0;

void escucharKernel(){
	log_debug(logger, "Entro hilo para escuchar kernel");
	//log_debug(logger, "socket memoria: %d", server_fd);
	sem_wait(&sem_cpu); //esperar que se conecte primero cpu
	t_socket socket_kernel = esperar_cliente(server_fd, logger);
	//log_debug(logger, "socket kernel: %d", socket_kernel);
	char* pid;
	tabla_segmentos tablaSegmentos;
	t_pedido_segmento* pedido;
	while(1){
		if(socket_kernel != -1){
			int cod_op = recibir_operacion(socket_kernel);
			switch (cod_op) {
			case TABLA_SEGMENTOS:
				tablaSegmentos = list_create();
				list_add(tablaSegmentos, segmento0);

				pid = recibirPID(socket_kernel);
				dictionary_put(diccionarioTablas, pid, tablaSegmentos);

				log_info(logger, "Enviando tabla de segmentos de proceso %s", pid);
				enviarSegmentosKernel(socket_kernel, tablaSegmentos);

				free(pid);
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
						log_info(logger, "Enviando segmentos a kernel...");
						enviarSegmentosKernel(socket_kernel, tablaSegmentos);
						break;
					case OUT_OF_MEMORY:
						log_error(logger,"Límite de segmentos máximos alcanzado - Limite: %d", cantidadMaxSegmentos);
						send(socket_kernel, &estadoCreacion, sizeof(op_code), 0);
						break;
					default:
						log_error(logger,"Error en obtener estado memoria");
						break;
					}
				}
				else
					estadoCreacion = LIMITE_SEGMENTOS_SUPERADO;
					//send(socket_kernel, &estadoCreacion, sizeof(op_code), 0);
				break;
			case DELETE_SEGMENT_OP:
				pedido = recibirPedidoSegmento(socket_kernel);
				eliminarSegmento(pedido);
				enviarSegmentosKernel(socket_kernel, tablaSegmentos);
				break;
			default:
				send(socket_kernel, (void *)(intptr_t)RESULT_ERROR, sizeof(uint32_t), (intptr_t)NULL);
				log_error(logger,"Se cerró la conexión");
				exit(1);
			}
		}else{
			log_error(logger,"Socket kernel == -1, la conexión se cerró");
		}
	}
}

void escucharCPU(){
	log_debug(logger, "Entro hilo para escuchar CPU");
	t_socket socket_cpu = esperar_cliente(server_fd, logger);
	sem_post(&sem_cpu);
	while(1){
		if(socket_cpu != -1){
			int cod_op = recibir_operacion(socket_cpu);
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

				//devuelvo el valor leido en la direccion pedida
				char* valor_leido = leer(direc_fisica, tamanio, pid);
				//hago el retardo que pide el enuncuado por acceder al espacio de memoria
				usleep(retardo_memoria * 1000);
				//envio el valor leido a cpu
				send(socket_cpu, valor_leido, tamanio, 0);
				log_debug(logger,"Lei el valor %s. Enviando a CPU...", valor_leido);
				free(valor_leido);
				break;
			case ESCRIBIR:
				char* valor;
				//recibo los datos para escritura
				buffer = recibir_buffer(&size, socket_cpu);
				memcpy(&pid, buffer + desplazamiento, sizeof(int));
				desplazamiento += sizeof(int);
				memcpy(&direc_fisica, buffer + desplazamiento, sizeof(u_int32_t));
				desplazamiento += sizeof(u_int32_t);
				memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
				desplazamiento += sizeof(int);
	     		memcpy(valor, buffer + desplazamiento, tamanio);

	     		log_debug(logger,"Me llego una escritura del valor %s en la direccion %d ", valor, direc_fisica);
				escribir(direc_fisica, tamanio, valor, pid);
				usleep(retardo_memoria * 1000);
				//le mando OK a cpu para que siga ejecutando
				send(socket_cpu, &RESULT_OK, tamanio, 0);
			}
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
