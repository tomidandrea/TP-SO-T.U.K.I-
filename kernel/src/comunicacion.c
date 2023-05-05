#include <comunicacion.h>
#include <utils.h>

extern t_config* config;
extern t_log* logger;

extern pthread_mutex_t mutex_procesos_new;
extern t_list* procesosNew;
extern sem_t sem_new_a_ready;

uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

int escucharConsolas(){
	t_list* lista = list_create();
	char* puerto = malloc(16);
	puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

	t_socket server_fd = iniciar_servidor(puerto, logger);
	printf("\nSocket conexion:%d \n",server_fd);

	free(puerto);

	t_socket socket_cliente = malloc(sizeof(int));
	while(1){
	socket_cliente = esperar_cliente(server_fd, logger); //Hace el accept

    printf("\nsocket conexion2:%d \n",server_fd);
		if(socket_cliente != -1){
				int cod_op = recibir_operacion(socket_cliente);
				switch (cod_op) {
						case PROGRAMA: //TODO

							lista = listaAInstrucciones(recibir_paquete(socket_cliente));

							//send(socket_cliente, &RESULT_OK, sizeof(int), NULL);

							log_info(logger, "Me llego un paquete\n");

							t_pcb* pcb = crearPCB(lista);
							pthread_mutex_lock(&mutex_procesos_new);
							list_add(procesosNew, pcb);
							pthread_mutex_unlock(&mutex_procesos_new);

							log_info(logger, "Se crea el proceso:%d en NEW", pcb->pid);
							sem_post(&sem_new_a_ready);

							break;
						case -1:
							send(socket_cliente, (void *)RESULT_ERROR, sizeof(int), NULL);
							log_error(logger, "el cliente se desconecto. Terminando servidor");
							//return EXIT_FAILURE;
							break;

						default:
							log_warning(logger,"Operacion desconocida. No quieras meter la pata");
							break;
						}
		}
	}
	return 0;
	//TODO: hacer que salga del while
	//send(socket_cliente, &RESULT_OK, sizeof(int), NULL);
	//close(socket_cliente);
	free(lista);
}

int escucharCPU(){
	char* puerto = malloc(16);
	puerto = config_get_string_value(config,"PUERTO_ESCUCHA");
	t_pcb* pcb;
	t_socket server_fd = iniciar_servidor(puerto, logger);
	free(puerto);

	t_socket socket_cliente = malloc(sizeof(int));
	while(1){
	socket_cliente = esperar_cliente(server_fd, logger); //Hace el accept

		if(socket_cliente != -1){
				int cod_op = recibir_operacion(socket_cliente);

				switch (cod_op) {

						case EXIT: //TODO
							pcb = recibir_proceso(socket_cliente);
							log_info(logger, "Me llego un PCB en exit\n");
							log_info(logger, "Aviso a memoria para liberar\n");
							//liberarRecurso(PCB);
							log_info(logger, "Libero memoria\n");
							//avisoConsolaLiberar();
							log_info(logger, "Aviso consola\n");
							//send(socket_cliente, &RESULT_OK, sizeof(int), NULL);


							break;
						case -1:
							send(socket_cliente, (void *)RESULT_ERROR, sizeof(int), NULL);
							log_error(logger, "el cliente se desconecto. Terminando servidor");
							//return EXIT_FAILURE;
							break;
						default:
							log_warning(logger,"Operacion desconocida. No quieras meter la pata");
							break;
						}
		}
	}
	return 0;
	//TODO: hacer que salga del while
	//send(socket_cliente, &RESULT_OK, sizeof(int), NULL);
	//close(socket_cliente);
	free(pcb);
}

void mandar_pcb_a_CPU(t_pcb* proceso){
	char* ip = config_get_string_value(config,"IP_CPU");
	char* puerto = config_get_string_value(config,"PUERTO_CPU");
	t_socket conexion = crear_conexion(ip, puerto, logger);

	// esto implementarlo para lo nuestro (copiado de consola)

	char* operacion;
	t_paquete *paquete = crear_paquete(PROCESO);
	int op_tamanio = 0;
	t_instruccion* inst = malloc(sizeof(t_instruccion));
	int cant = list_size(proceso->instrucciones);
	int cant_parametros = 0;

	agregar_valor_estatico(paquete, &(proceso -> pid));
	agregar_valor_estatico(paquete, &(proceso -> pc));
	agregar_a_paquete(paquete, proceso -> registros->AX, 4);
	agregar_a_paquete(paquete, proceso -> registros->BX, 4);
	agregar_a_paquete(paquete, proceso -> registros->CX, 4);
	agregar_a_paquete(paquete, proceso -> registros->DX, 4);
	agregar_a_paquete(paquete, proceso -> registros->EAX, 8);
	agregar_a_paquete(paquete, proceso -> registros->EBX, 8);
	agregar_a_paquete(paquete, proceso -> registros->ECX, 8);
	agregar_a_paquete(paquete, proceso -> registros->EDX, 8);
	agregar_a_paquete(paquete, proceso -> registros->RAX, 16);
	agregar_a_paquete(paquete, proceso -> registros->RBX, 16);
	agregar_a_paquete(paquete, proceso -> registros->RCX, 16);
	agregar_a_paquete(paquete, proceso -> registros->RDX, 16);

	//TODO: tabla de segmentos

	for(int i = 0;i<cant;i++) {

	    inst = list_get(proceso -> instrucciones,i);
	    operacion = string_duplicate(inst -> instruccion);
	    op_tamanio = strlen(operacion)+1;

	    agregar_a_paquete(paquete,operacion,op_tamanio);

	    cant_parametros = cantParametros(operacion);
	    for(int i=0; i<cant_parametros; i++) {
	    	agregar_a_paquete(paquete,inst->parametros[i],strlen(inst->parametros[i])+1);
	    }

	}

	enviar_paquete(paquete,conexion);                // serializa el paquete y lo envia

	eliminar_paquete(paquete);                //elimina el paquete y lo que contiene

}


