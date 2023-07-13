#include "utilsCpu.h"

extern t_registros* registros;
extern t_socket conexionMemoria;
extern t_log* logger;
extern u_int32_t direcFisicaAEnviar;

t_pcb* recibir_proceso(int socket_cliente) {
	    int size;
		int desplazamiento = 0;
		void * buffer;
		t_list* valores = list_create();
		int tamanio;
		int tamanio_tabla;
        t_pcb* pcb = inicializar_pcb();

		//printf("hola, recibiendo proceso\n");

		//printf("recibiendo buffer\n");

		buffer = recibir_buffer(&size, socket_cliente);

		//recibir_variable(&pid,buffer,&desplazamiento);      // recibo pid
		//pcb->pid = pid;                                     // actualizo pid en el pcb
		memcpy(&(pcb->pid), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		/*recibir_variable(&pc,buffer,&desplazamiento);       //recivo pc
		pcb->pc = pc;                                      // actualizo pc en el pcb
*/
		memcpy(&(pcb->pc), buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		memcpy(&(tamanio_tabla), buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);

			for(int i=0; i<tamanio_tabla;i++){
				t_segmento* segmento = malloc(sizeof(t_segmento));
				memcpy(&(segmento->id), buffer + desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				memcpy(&(segmento->base), buffer + desplazamiento, sizeof(u_int32_t));
				desplazamiento+=sizeof(u_int32_t);
				memcpy(&(segmento->limite), buffer + desplazamiento, sizeof(u_int32_t));
				desplazamiento+=sizeof(u_int32_t);
				list_add(pcb->tablaSegmentos, segmento);
				t_segmento* seg = list_get(pcb->tablaSegmentos, i);
				printf("PID %d: Segmento %d - base: %d - limite: %d\n", pcb->pid, seg->id, seg->base, seg->limite); //Aca los muestra bien los segmentos
			}

		/* alternativa con vectores por tamaño
		recibir_registros(buffer,&desplazamiento, 4,registros_>tamanio_4);
		pcb->registros->tamanio_4 = registros->tamanio_4;
		recibir_registros(buffer,&desplazamiento, 8,registros_>tamanio_8);
		pcb->registros->tamanio_8 = registros->tamanio_8;
		recibir_registros(buffer,&desplazamiento, 16,registros_>tamanio_16);
		pcb->registros->tamanio_16 = registros->tamanio_16;
		*/

		while(desplazamiento < size)                                           //recibo todos los registros y las instrucciones y los meto en una lista de strings
		{
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			char* valor = malloc(tamanio);
			memcpy(valor, buffer+desplazamiento, tamanio);
			desplazamiento+=tamanio;
			list_add(valores, valor);
        }


		t_list* lista_registros = list_take_and_remove(valores,12);     // saco los primeros 12 elementos de la lista de strings que serian los 12 registros y los agrego en una nueva lista. En la anterior lista solo queadn los strings de las instrucciones.
		actualizar_registros_cpu(pcb,lista_registros);                          // actualizo los registros de la cpu
		t_list* instrucciones = listaAInstrucciones(valores);              // paso de lista de strings a lista de instrucciones
	    pcb->instrucciones = instrucciones;                             // actualizo lista de instrucciones en el pcb
	    list_destroy_and_destroy_elements(lista_registros, free);
	    list_destroy_and_destroy_elements(valores, free);
	    //list_destroy(instrucciones);
	    free(buffer);

	    return pcb;
}

void recibir_variable(int* variable, t_buffer*buffer,int* desplazamiento) {

	memcpy(&variable, buffer + (*desplazamiento), sizeof(int));
	(*desplazamiento)+=sizeof(int);
}

void actualizar_registros_cpu(t_pcb *pcb, t_list* lista_registros) {
	/*int cant = list_size(lista_registros);
	for(int i = 0;i<cant;i++) {
		char aux[4];
		char* auxp;
		auxp = list_get(lista_registros,i);
		strncpy(aux, (char *)list_get(lista_registros,i),4);
		log_info(logger, "elemento: %s \n", auxp);
		log_info(logger, "tamanio: %d \n", sizeof(*aux));
		log_info(logger, "tamanio: %d \n", sizeof(aux));
	}*/
	memcpy(registros->AX, list_get(lista_registros,0), 4);
	memcpy(registros->BX, list_get(lista_registros,1),4);
	memcpy(registros->CX, list_get(lista_registros,2),4);
	memcpy(registros->DX, list_get(lista_registros,3),4);
	memcpy(registros->EAX,list_get(lista_registros,4),8);
	memcpy(registros->EBX,list_get(lista_registros,5),8);
	memcpy(registros->ECX,list_get(lista_registros,6),8);
	memcpy(registros->EDX,list_get(lista_registros,7),8);
	memcpy(registros->RAX,list_get(lista_registros,8),16);
	memcpy(registros->RBX,list_get(lista_registros,9),16);
	memcpy(registros->RCX,list_get(lista_registros,10),16);
	memcpy(registros->RDX,list_get(lista_registros,11),16);
}


void actualizar_registros_pcb(t_pcb *pcb) {

	strcpy(pcb->registros->AX,registros->AX);
	strcpy(pcb->registros->BX, registros->BX);
	strcpy(pcb->registros->CX, registros->CX);
	strcpy(pcb->registros->DX, registros->DX);
	strcpy(pcb->registros->EAX, registros->EAX);
	strcpy(pcb->registros->EBX, registros->EBX);
	strcpy(pcb->registros->ECX, registros->ECX);
	strcpy(pcb->registros->EDX, registros->EDX);
	strcpy(pcb->registros->RAX, registros->RAX);
	strcpy(pcb->registros->RBX, registros->RBX);
	strcpy(pcb->registros->RCX, registros->RCX);
	strcpy(pcb->registros->RDX, registros->RDX);
}

/* alternativa con vectores segunel tamaño
void recibir_registros(t_buffer*buffer, int* desplazamiento, int tamanio_registro, char**registro) {

	for(int i = 0; i<4;i++) {
		char valor[tamanio_registro];
		memcpy(valor, buffer + (*desplazamiento), tamanio_registro);
		(*desplazamiento)+=tamanio_registro;
		registro[4][i] = string_duplicate(valor);
	}

}
*/


void enviar_contexto(t_pcb* proceso, t_instruccion* inst, int conexion){

	t_paquete *paquete = crear_paquete(CONTEXTO);
	int cant_parametros = cantParametros(inst->instruccion);
	t_segmento* segmento;

	agregar_valor_estatico(paquete, &(proceso -> pid));
	agregar_valor_estatico(paquete, &(proceso -> pc));
	agregar_valor_estatico(paquete, &(proceso -> motivo));
	agregar_valor_uint(paquete, &direcFisicaAEnviar);
	agregar_valor_estatico(paquete, &(cant_parametros));
	for(int i=0; i<cant_parametros; i++) {
		agregar_a_paquete(paquete,inst->parametros[i],strlen(inst->parametros[i])+1);
	}
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

	int cantidad = list_size(proceso->tablaSegmentos);
		agregar_valor_estatico(paquete,&cantidad);
		for (int i = 0; i<cantidad; i++){
			segmento = list_get(proceso->tablaSegmentos, i);
			agregar_valor_estatico(paquete,&(segmento->id));
			agregar_valor_uint(paquete,&(segmento->base));
			agregar_valor_uint(paquete,&(segmento->limite));
	}

	enviar_paquete(paquete,conexion);                // serializa el paquete y lo envia

	eliminar_paquete(paquete);                //elimina el paquete y lo que contiene

}
//solo para mov_out
void escribir_memoria(int pid, u_int32_t direc_fisica,char* valor, int tamanio_valor) {
	t_paquete *paquete = crear_paquete(ESCRIBIR);

	agregar_valor_estatico(paquete, &pid);
	agregar_valor_uint(paquete, &(direc_fisica));
	agregar_a_paquete(paquete, valor, tamanio_valor);

	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);

	int cod_op;
	char* mensaje;
	if(recv(conexionMemoria, &cod_op, sizeof(int), MSG_WAITALL) > 0){
		 mensaje=recibir_mensaje(conexionMemoria, logger);
			log_debug(logger,"Me llego de memoria el resultado: %s",mensaje);
		} else {
			log_error(logger,"No me llego el resultado de memoria");
		}
	free(mensaje);
}

//solo para mov_in

char* leer_memoria(int pid, u_int32_t direc_fisica, int tamanio_a_leer) {
	t_paquete *paquete = crear_paquete(LEER);

	agregar_valor_estatico(paquete, &pid);
	agregar_valor_uint(paquete, &(direc_fisica));
	agregar_valor_estatico(paquete, &(tamanio_a_leer));

	enviar_paquete(paquete,conexionMemoria);
	eliminar_paquete(paquete);
	//hago el recv y devuelvo el valor leido
	char* valor_leido;
	int cod_op;
	if(conexionMemoria!=-1){
		cod_op = recibir_operacion(conexionMemoria);
		if(cod_op==0)
			log_debug(logger,"codOP: MENSAJE");

		valor_leido = recibir_mensaje(conexionMemoria, logger); //tiene el \0

	} else {
		log_error(logger,"No me llego el resultado de memoria");
	}
	return valor_leido;
}

void liberar_proceso(t_pcb* pcb) {
	liberar_instrucciones(pcb->instrucciones);
	list_destroy(pcb->instrucciones);
	free(pcb->registros);
	temporal_destroy(pcb->tiempoEnReady);
	temporal_destroy(pcb->tiempoCPU);
	list_destroy(pcb->archivosAbiertos);
	list_destroy_and_destroy_elements(pcb->tablaSegmentos, free);
	free(pcb);
}



