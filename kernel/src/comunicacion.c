#include <comunicacion.h>
#include <utils.h>

extern t_config* config;
extern t_log* logger;
extern t_socket conexionCPU;

extern pthread_mutex_t mutex_procesos_new;
extern t_list* procesosNew;
extern sem_t sem_new_a_ready;


uint32_t RESULT_OK = 0;
uint32_t RESULT_ERROR = 1;

int escucharConsolas(){
	t_list* lista;// = list_create(); ya hacemos el create en listaAInstruccion
	char* puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

	t_socket server_fd = iniciar_servidor(puerto, logger);
	printf("\nSocket conexion:%d \n",server_fd);

	free(puerto);

	//t_socket socket_cliente = malloc(sizeof(t_socket));
	t_socket socket_cliente;
	while(1){
	socket_cliente = esperar_cliente(server_fd, logger); //Hace el accept
    printf("\nsocket cliente:%d \n",socket_cliente);
    //TODO: deberiamos agregar el socket cliente para saber que Consola finalizar
		if(socket_cliente != -1){
				int cod_op = recibir_operacion(socket_cliente);
				switch (cod_op) {
						case PROGRAMA: //TODO
							t_list* buffer = recibir_paquete(socket_cliente);
							lista = listaAInstrucciones(buffer);
							list_destroy_and_destroy_elements(buffer, free);
							//send(socket_cliente, &RESULT_OK, sizeof(int), NULL);

							log_info(logger, "Me llego un paquete\n");

							t_pcb* pcb = crearPCB(lista); //agregar para pasar el socket de consola
							liberar_instrucciones(lista); // dentro de crearPCB estamos duplicando la lista, entonces libero la que ya no usamos
							list_destroy(lista);
							pthread_mutex_lock(&mutex_procesos_new);
							list_add(procesosNew, pcb);
							pthread_mutex_unlock(&mutex_procesos_new);

							log_info(logger, "Se crea el proceso:%d en NEW", pcb->pid);
							sem_post(&sem_new_a_ready);

							break;
						case -1:
							send(socket_cliente, (void *)(intptr_t)RESULT_ERROR, sizeof(uint32_t), (intptr_t)NULL);
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


void mandar_pcb_a_CPU(t_pcb* proceso){
	char* operacion;
	t_paquete *paquete = crear_paquete(PROCESO);
	int op_tamanio = 0;

	int cant = list_size(proceso->instrucciones);
	int cant_parametros = 0;
	log_info(logger, "PID:%d\n",proceso->pid);
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

	t_instruccion* inst;
	for(int i = 0;i<cant;i++) {
		printf("Instruccion: %s\n", ((t_instruccion*)list_get(proceso -> instrucciones,i))->instruccion);
	    inst = list_get(proceso -> instrucciones,i);
	    printf("Instruccion: %s\n",inst->instruccion);
	    operacion = copiar(inst->instruccion);
		//operacion = ((t_instruccion)list_get(proceso -> instrucciones,i))->instruccion;
	    op_tamanio = strlen(operacion)+1;

	    agregar_a_paquete(paquete,operacion,op_tamanio);

	    cant_parametros = cantParametros(operacion);
	    //inst = inicializar_instruccion(cant_parametros);
	    free(operacion);
	    for(int i=0; i<cant_parametros; i++) {
	    	agregar_a_paquete(paquete,inst->parametros[i],strlen(inst->parametros[i])+1);
	    }
	}

	enviar_paquete(paquete,conexionCPU);                // serializa el paquete y lo envia

	eliminar_paquete(paquete);                //elimina el paquete y lo que contiene

}


