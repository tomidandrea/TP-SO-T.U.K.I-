#include "sockets.h"

t_socket iniciarConexion(t_config* config, t_log*logger, char*ip_config, char*puerto_config){
	char* ip = config_get_string_value(config,ip_config);
	char* puerto = config_get_string_value(config,puerto_config);
	t_socket conexion = crear_conexion(ip, puerto, logger);
	return conexion;
}

t_socket iniciarServidor(t_config*config, t_log* logger, char* puerto_config) {
    char* puerto;
    puerto = config_get_string_value(config,puerto_config);

    t_socket servidor = iniciar_servidor(puerto, logger);
    free(puerto);
    return servidor;
}

t_socket crear_conexion(char* ip, char* puerto, t_log* logger)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
            server_info->ai_socktype,
            server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	log_info(logger, "Inicio conexion \n");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void enviar_mensaje2(char* mensaje, int socket_cliente)
{
	//t_paquete* paquete = malloc(sizeof(t_paquete));
	printf("mensaje: %s\n", mensaje);
	/*char a_enviar[5];
	strcpy(a_enviar, mensaje);
	for (int i = 0; i < 5; ++i) {
		printf("%c",a_enviar[i]);
	}
	printf("\n");*/
	//paquete->codigo_operacion = MENSAJE;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = strlen(mensaje) + 1;
	printf("size: %d\n", buffer->size);
	buffer->stream = malloc(buffer->size);
	memcpy(buffer->stream, mensaje, buffer->size);
	printf("mensaje stream: %s\n", buffer->stream);
	//int bytes = buffer->size + 2*sizeof(int);
	int bytes = buffer->size + sizeof(int);

	//void* a_enviar = serializar_paquete(paquete, bytes);

	void * magic = malloc(bytes);
	printf("1. %p\n", magic);
		int desplazamiento = 0;
		//int num = 4;
		//memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
		//desplazamiento+= sizeof(int);
		memcpy(magic + desplazamiento, &(buffer->size), sizeof(int));
		desplazamiento+= sizeof(int);
		memcpy(magic + desplazamiento, buffer->stream, buffer->size);
		//desplazamiento+= buffer->size;
		//memcpy(magic + desplazamiento, &(num), sizeof(int));
		//agregar_a_paquete(paquete, valor, tamanio)
		//memcpy(buffer->stream, &(buffer->size), sizeof(int));
		//memcpy(buffer->stream + sizeof(int), mensaje, buffer->size);

		printf("2. %p\n", magic);
		//buffer->size += tamanio + sizeof(int);

		//return magic;
		int size;
		char* valor;

		memcpy(&size, magic, sizeof(int));
		printf("size obtenido: %d\n", size);
		valor = malloc(size);
		memcpy(valor, magic+sizeof(int),size);
		printf("mensaje: %s\n", valor);
		printf("sizeof magic %lu\n", sizeof(magic));
		printf("bytes %d\n", bytes);

	send(socket_cliente, magic, bytes, 0);

	//free(magic);
	//eliminar_paquete(paquete);
	//free(buffer->stream);
	//free(buffer);
}

char* recibir_mensaje(int socket_cliente, t_log* logger) //agrego que mande logger como parametro
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	return buffer;
}

char* recibir_mensaje2(int socket_cliente, t_log* logger) //agrego que mande logger como parametro
{
	//char* buffer = recibir_buffer(&size, socket_cliente);
	void* magic = malloc(5*sizeof(char) + sizeof(int));
	printf("3. %p\n", magic);
	printf("sizeof magic %lu\n", sizeof(magic));
	int size;
	char* buffer;
	int desplazamiento= 0;
	int num;
			recv(socket_cliente, magic, 5*sizeof(char) + sizeof(int), MSG_WAITALL);
			printf("4. %p\n", magic);
			memcpy(&size, magic, sizeof(int));
			desplazamiento += sizeof(int);
			buffer = malloc(size);
			/*for(int i =0; i<size; i++){
				memcpy(buffer[i], magic + desplazamiento, sizeof(char));
				desplazamiento+=i;
				printf("c%d: %c\n",i,buffer[i]);
			}*/
			memcpy(buffer, magic + desplazamiento, 5*sizeof(char));
			//memcpy(&num, magic + desplazamiento, sizeof(int));
	log_info(logger, "Me llego el mensaje %s de tamaño %d", buffer, size);
	//log_info(logger, "Me llego el num %d", num);
	return buffer;

	/*int size;
		char* buffer = recibir_buffer(&size, socket_cliente);
		log_info(logger, "Me llego el mensaje %s", buffer);
		return buffer;*/
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

/*
 *
 * Aca empieza utils de servidor
 */

t_socket iniciar_servidor(char* puerto, t_log* logger)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;//, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor

	// Asociamos el socket a un puerto
	socket_servidor = socket(servinfo->ai_family,
			servinfo->ai_socktype,
			servinfo->ai_protocol);

	// Escuchamos las conexiones entrantes

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_info(logger, "Servidor listo para recibir al cliente");
	//log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

t_socket esperar_cliente(int socket_servidor, t_log* logger)
{
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(t_socket socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

		recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
		buffer = malloc(*size);
		recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	//printf("hola, recibiendo paquete\n");

	buffer = recibir_buffer(&size, socket_cliente);

	//printf("recibiendo buffer\n");
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}


t_contexto* recibir_contexto(int socket_cliente) {
	    int size;
		int desplazamiento = 0;
		void * buffer;
		t_list* valores = list_create();
		int tamanio;
		int tamanio_tabla;
		t_segmento* segmento = malloc(sizeof(t_segmento));
        t_contexto* contexto = inicializar_contexto();

		buffer = recibir_buffer(&size, socket_cliente);

		// Pasamos el PID solo para confirmación del proceso.
		memcpy(&(contexto->pid), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		memcpy(&(contexto->pc), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		memcpy(&(contexto->motivo), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		memcpy(&(contexto->cantidadParametros), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		int cantParametros = contexto->cantidadParametros;
		contexto->parametros = inicializar_parametros(cantParametros);
		for(int i = 0;i<cantParametros; i++){
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			char* valor = malloc(tamanio);
			memcpy(valor, buffer+desplazamiento, tamanio);
			desplazamiento+=tamanio;
			contexto->parametros[i] = copiar(valor);
		 }


		//while(desplazamiento < size)
		for(int i=0; i< 12;i++)
		{
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			char* valor = malloc(tamanio);
			memcpy(valor, buffer+desplazamiento, tamanio);
			desplazamiento+=tamanio;
			list_add(valores, valor);
        }

		strncpy(contexto->registros->AX, list_get(valores,0), 4);
		strncpy(contexto->registros->BX, list_get(valores,1), 4);
		strncpy(contexto->registros->CX, list_get(valores,2), 4);
		strncpy(contexto->registros->DX, list_get(valores,3), 4);
		strncpy(contexto->registros->EAX, list_get(valores,4), 8);
		strncpy(contexto->registros->EBX, list_get(valores,5), 8);
		strncpy(contexto->registros->ECX, list_get(valores,6), 8);
		strncpy(contexto->registros->EDX, list_get(valores,7), 8);
		strncpy(contexto->registros->RAX,list_get(valores,8), 16);
		strncpy(contexto->registros->RBX, list_get(valores,9), 16);
		strncpy(contexto->registros->RCX, list_get(valores,10), 16);
		strncpy(contexto->registros->RDX, list_get(valores,11), 16);

		memcpy(&(tamanio_tabla), buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);

			for(int i=0; i<tamanio_tabla;i++){
				memcpy(&(segmento->id), buffer + desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				memcpy(&(segmento->base), buffer + desplazamiento, sizeof(u_int32_t));
				desplazamiento+=sizeof(u_int32_t);
				memcpy(&(segmento->limite), buffer + desplazamiento, sizeof(u_int32_t));
				desplazamiento+=sizeof(u_int32_t);
				list_add(contexto->tablaSegmentos, segmento);
			}

		free(buffer);
		list_destroy_and_destroy_elements(valores, free);

	    return contexto;
}

t_list* recibirTablaSegmentos(t_socket socket_memoria){
	void * buffer;
	int size;
	int tamanio_tabla;
	int desplazamiento = 0;
	t_segmento* segmento = malloc(sizeof(t_segmento));
	t_list* tabla = list_create();

	buffer = recibir_buffer(&size, socket_memoria);
	memcpy(&(tamanio_tabla), buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	for(int i=0; i<tamanio_tabla;i++){
		memcpy(&(segmento->id), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(&(segmento->base), buffer + desplazamiento, sizeof(u_int32_t));
		desplazamiento+=sizeof(int);
		memcpy(&(segmento->limite), buffer + desplazamiento, sizeof(u_int32_t));
		desplazamiento+=sizeof(int);
		list_add(tabla, segmento);
		t_segmento* seg = list_get(tabla, i);
		printf("Segmento %d\n", seg->id);
	}
	for(int i=0; i<list_size(tabla);i++){
		t_segmento* seg = list_get(tabla, i);
		printf("Segmento %d\n", seg->id);
	}
	return tabla;

}

t_segmento* recibirSegmento(t_socket socket_memoria){
	void * buffer;
	int size;
	int desplazamiento = 0;
	t_segmento* segmento = malloc(sizeof(t_segmento));

	buffer = recibir_buffer(&size, socket_memoria);

	memcpy(&(segmento->id), buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(&(segmento->base), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento+=sizeof(int);
	memcpy(&(segmento->limite), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento+=sizeof(int);

	return segmento;
}

t_pedido_segmento* recibirPedidoSegmento(t_socket socket_kernel){
	void * buffer;
	int size;
	int desplazamiento = 0;
	t_pedido_segmento* pedido = malloc(sizeof(t_pedido_segmento));
	buffer = recibir_buffer(&size, socket_kernel);
	memcpy(&(pedido->pid), buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	memcpy(&(pedido->id_segmento), buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	memcpy(&(pedido->tamanio), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento+=sizeof(u_int32_t);

	free(buffer);

	return pedido;
}

char* recibirPID(t_socket socket){
	void * buffer;
	int size;
	int desplazamiento = 0;
	int pid_int;
	char* pid;

	buffer = recibir_buffer(&size, socket);

	memcpy(&(pid_int), buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	pid = string_itoa(pid_int);

	free(buffer);
	return pid;
}







