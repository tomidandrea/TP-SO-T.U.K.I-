#include "sockets.h"

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

t_socket iniciar_servidor(char* puerto, t_log* logger) //agrego que mande logger como parametro
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");

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

t_socket esperar_cliente(int socket_servidor, t_log* logger) //agrego que mande logger como parametro
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");

	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
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

void recibir_mensaje(int socket_cliente, t_log* logger) //agrego que mande logger como parametro
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
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
        t_contexto* contexto = inicializar_contexto();

		buffer = recibir_buffer(&size, socket_cliente);

		// Pasamos el PID solo para confirmación del proceso.
		memcpy(&(contexto->pid), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		memcpy(&(contexto->pc), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		memcpy(&(contexto->motivo), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		while(desplazamiento < size)
		{
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			char* valor = malloc(tamanio);
			memcpy(valor, buffer+desplazamiento, tamanio);
			desplazamiento+=tamanio;
			list_add(valores, valor);
        }

		strcpy(contexto->registros->AX, (char *) list_get(valores,0));
		strcpy(contexto->registros->BX , (char *) list_get(valores,1));
		strcpy(contexto->registros->CX, list_get(valores,2));
		strcpy(contexto->registros->DX, list_get(valores,3));
		strcpy(contexto->registros->EAX, list_get(valores,4));
		strcpy(contexto->registros->EBX, list_get(valores,5));
		strcpy(contexto->registros->ECX, list_get(valores,6));
		strcpy(contexto->registros->EDX, list_get(valores,7));
		strcpy(contexto->registros->RAX,list_get(valores,8));
		strcpy(contexto->registros->RBX, list_get(valores,9));
		strcpy(contexto->registros->RCX, list_get(valores,10));
		strcpy(contexto->registros->RDX, list_get(valores,11));

		free(buffer);
		list_destroy_and_destroy_elements(valores, free);

	    return contexto;
}






