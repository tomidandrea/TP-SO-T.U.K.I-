#include "utilsMemoria.h"

extern void* espacioMemoria;
extern t_log* logger;
t_dictionary* diccionarioTablas;
t_segmento* segmento0;


//TODO: tendria que ser existir una lista de tablas de segmentos o un diccionario (preferiblemente)
extern t_config* config;


void inicializarEstructuras(){

	diccionarioTablas = dictionary_create();

	segmento0 = malloc(sizeof(t_segmento));
	int tamanioMemoria = config_get_int_value(config,"TAM_MEMORIA");
	int tamanioSegmento0 = config_get_int_value(config,"TAM_SEGMENTO_0");
	int cantidadSegmentos = config_get_int_value(config,"CANT_SEGMENTOS");

	espacioMemoria = malloc(tamanioMemoria);
	segmento0->base = 0;
	segmento0->id = 0;
	segmento0->limite = tamanioSegmento0;
}

void enviarSegmentosKernel(t_socket socket_kernel, tabla_segmentos tablaSegmentos){

	t_paquete* paquete = crear_paquete(TABLA_SEGMENTOS);
	t_segmento* segmento;
	int cantidad = list_size(tablaSegmentos);
	agregar_valor_estatico(paquete,&cantidad);
	for (int i = 0; i<cantidad; i++){
		segmento = list_get(tablaSegmentos, i);
		agregar_valor_estatico(paquete,&(segmento->id));
		agregar_valor_uint(paquete,&(segmento->base));
		agregar_valor_uint(paquete,&(segmento->limite));
	}

	//TODO Ver estas conxiones
	enviar_paquete(paquete, socket_kernel);
	eliminar_paquete(paquete);
}

void* leer(u_int32_t direc, int tamanio, int pid) {
	void* valor = malloc(tamanio);
	memcpy(valor, espacioMemoria + direc, tamanio);
	log_info(logger,  "PID: %d- Acción: LEER - Dirección física: %d - Tamaño: %d- Origen: CPU", pid, direc, tamanio);

	return valor;
}

void escribir(u_int32_t direc, int tamanio, char* valor, int pid) {

	memcpy(espacioMemoria + direc, valor, tamanio);
	log_info(logger,  "PID: %d- Acción: ESCRIBIR - Dirección física: %d - Tamaño: %d- Origen: CPU", pid, direc, tamanio);
}


