#include "utilsCpu.h"

extern t_registros* registros;

t_pcb* recibir_proceso(int socket_cliente) {
	    int size;
		int desplazamiento = 0;
		void * buffer;
		t_list* valores = list_create();
		int tamanio;
        t_pcb* pcb = inicializar_pcb();
		t_list* instrucciones = list_create();

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

		/* alternativa con vectores por tamaño
		recibir_registros(buffer,&desplazamiento, 4,registros_>tamanio_4);
		pcb->registros->tamanio_4 = registros->tamanio_4;
		recibir_registros(buffer,&desplazamiento, 8,registros_>tamanio_8);
		pcb->registros->tamanio_8 = registros->tamanio_8;
		recibir_registros(buffer,&desplazamiento, 16,registros_>tamanio_16);
		pcb->registros->tamanio_16 = registros->tamanio_16;
		*/

		while(desplazamiento < size)                                           //recivo todos los registros y las instrucciones y los meto en una lista de strings
		{
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			char* valor = malloc(tamanio);
			memcpy(valor, buffer+desplazamiento, tamanio); //rompe aca
			desplazamiento+=tamanio;
			list_add(valores, valor);
        }


		t_list* lista_registros = list_take_and_remove(valores,12);     // saco los primeros 12 elementos de la lista de strings que serian los 12 registros y los agrego en una nueva lista. En la anterior lista solo queadn los strings de las instrucciones.
		actualizar_registros_cpu(pcb,lista_registros);                          // actualizo los registros de la cpu
		instrucciones = listaAInstrucciones(valores);              // paso de lista de strings a lista de instrucciones
	    pcb->instrucciones = instrucciones;                             // actualizo lista de instrucciones en el pcb

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
	strcpy(registros->AX, (char *) list_get(lista_registros,0));
	strcpy(registros->BX , (char *) list_get(lista_registros,1));
	strcpy(registros->CX, list_get(lista_registros,2));
	strcpy(registros->DX, list_get(lista_registros,3));
	strcpy(registros->EAX, list_get(lista_registros,4));
	strcpy(registros->EBX, list_get(lista_registros,5));
	strcpy(registros->ECX, list_get(lista_registros,6));
	strcpy(registros->EDX, list_get(lista_registros,7));
	strcpy(registros->RAX, list_get(lista_registros,8));
	strcpy(registros->RBX, list_get(lista_registros,9));
	strcpy(registros->RCX, list_get(lista_registros,10));
	strcpy(registros->RDX, list_get(lista_registros,11));
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

	agregar_valor_estatico(paquete, &(proceso -> pid));
	agregar_valor_estatico(paquete, &(proceso -> pc));
	agregar_valor_estatico(paquete, &(proceso -> motivo));
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

	//TODO: tabla de segmentos

	enviar_paquete(paquete,conexion);                // serializa el paquete y lo envia

	eliminar_paquete(paquete);                //elimina el paquete y lo que contiene

}


