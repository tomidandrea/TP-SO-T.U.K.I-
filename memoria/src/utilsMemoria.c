#include "utilsMemoria.h"

extern void* espacioMemoria;
extern tabla_segmentos tabla_huecos;
extern t_log* logger;
t_dictionary* diccionarioTablas;
t_segmento* segmento0;

#define NO_HAY_HUECO_ASIGNABLE -1

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

	tabla_huecos = inicializarTablaHuecosLibres(tamanioMemoria,tamanioSegmento0) ;
}

tabla_segmentos inicializarTablaHuecosLibres(int tamanioMemoria,int tamanioSegmento0){
	//iniciamos la tabla con un hueco libre, a partir de donde termina el seg0 hasta el tamaño de la memoria
	t_segmento* hueco = malloc(sizeof(t_segmento));
	hueco->id = 0;
	hueco->base = tamanioSegmento0; //TODO alejo: ver si rompe por el tipo de dato
	hueco->limite = tamanioMemoria;

	tabla_segmentos tabla_segmentos = list_create();
	list_add(tabla_segmentos, hueco);
	return tabla_segmentos;
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

u_int32_t obtenerTamanioSegmento(t_segmento* segmento){
	return segmento->limite - segmento->base;
}

int hayEspacio(t_pedido_segmento* pedido){
	t_segmento* hueco;
	u_int32_t tamanio;
	int cantidadHuecos = list_size(tabla_huecos);
	for(int i=0;i<cantidadHuecos;i++){ //por ahora esto solo funciona con FIRST FIT
		hueco = list_get(tabla_huecos, i);
		tamanio=obtenerTamanioSegmento(hueco);
		if(pedido->tamanio < tamanio){
			log_info(logger, "Hueco libre disponible");
			return hueco->id;
		}
	}
	return NO_HAY_HUECO_ASIGNABLE;
}

void crearSegmento(t_pedido_segmento* pedido) {
	int hueco_id = hayEspacio(pedido);
	if(hueco_id!=-1){ //hay hueco disponible
		t_segmento* hueco = list_get(tabla_huecos, hueco_id);
		char* pid = string_itoa(pedido->pid);
		tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, pid);

		t_segmento* nuevoSegmento = malloc(sizeof(t_segmento));
		nuevoSegmento->base = hueco->base;
		nuevoSegmento->id = pedido->id_segmento;
		nuevoSegmento->limite = hueco->base + pedido->tamanio;

		list_add(tabla_del_proceso, nuevoSegmento);
		log_info(logger, "Nuevo segmento: id %d, base %d, limite %d", nuevoSegmento->id, nuevoSegmento->base, nuevoSegmento->limite);

		hueco->base = nuevoSegmento->limite;
	}else{ //TODO: si no hay hueco

		log_error(logger, "No hay hueco libre disponible");
	}
}


