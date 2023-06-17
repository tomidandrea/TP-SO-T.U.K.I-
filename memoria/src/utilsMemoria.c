#include "utilsMemoria.h"

extern void* espacioMemoria;
extern tabla_segmentos tabla_huecos;
extern t_log* logger;
extern t_config* config;
int cantidadMaxSegmentos;
t_dictionary* diccionarioTablas;
t_segmento* segmento0;
int huecoDisponible;
op_code estadoCreacion;
//TODO alejiti: ver si necesitamos mutex para el diccionario y tabla de huecos

t_segmento* crear_t_segmento(int id, u_int32_t base, u_int32_t limite){
	t_segmento* segmento = malloc(sizeof(t_segmento));
	segmento->id = id;
	segmento->base = base;
	segmento->limite = limite;
	return segmento;
}

void inicializarEstructuras(){

	diccionarioTablas = dictionary_create();

	int tamanioMemoria = config_get_int_value(config,"TAM_MEMORIA");
	int tamanioSegmento0 = config_get_int_value(config,"TAM_SEGMENTO_0");
	cantidadMaxSegmentos = config_get_int_value(config,"CANT_SEGMENTOS");

	espacioMemoria = malloc(tamanioMemoria);

	segmento0 = crear_t_segmento(0, 0, tamanioSegmento0);

	tabla_huecos = inicializarTablaHuecosLibres(tamanioMemoria,tamanioSegmento0) ;
}

tabla_segmentos inicializarTablaHuecosLibres(int tamanioMemoria,int tamanioSegmento0){
	//iniciamos la tabla con un hueco libre, a partir de donde termina el seg0 hasta el tamaño de la memoria
	t_segmento* hueco = crear_t_segmento(0, tamanioSegmento0, tamanioMemoria);
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
	enviar_paquete(paquete, socket_kernel);
	eliminar_paquete(paquete);
}

u_int32_t obtenerTamanioSegmento(t_segmento* segmento){
	return segmento->limite - segmento->base;
}

int hayEspacio(t_pedido_segmento* pedido){
	t_segmento* hueco;
	u_int32_t tamanio;
	u_int32_t tamanioTotal = 0; // acumula el tamaño de los huecos
	int cantidadHuecos = list_size(tabla_huecos);

	log_debug(logger, "Llegó un pedido de segmento: Id: %d - Tamaño: %d", pedido->id_segmento, pedido->tamanio);

	//TODO alejiti: switch segun el algoritmo de asignacion en config
	for(int i=0;i<cantidadHuecos;i++){ //por ahora esto solo funciona con FIRST FIT
		hueco = list_get(tabla_huecos, i);
		tamanio=obtenerTamanioSegmento(hueco);
		log_debug(logger, "Hueco %d tiene tamaño: %d", i, tamanio);
		if(pedido->tamanio <= tamanio){
			log_info(logger, "Hueco libre disponible");
			huecoDisponible = hueco->id; // guardo el hueco en una variable global
			return HAY_HUECO_ASIGNABLE;
		}else{
			tamanioTotal += tamanio;
		}
	}
	if(tamanioTotal > pedido->tamanio){
		log_debug(logger, "Hay espacio si compactamos");
		return HAY_ESPACIO_AL_COMPACTAR;
	}
	return NO_HAY_HUECO_ASIGNABLE;
}

void crearSegmento(t_pedido_segmento* pedido) {
	int estadoEspacio = hayEspacio(pedido);
	switch(estadoEspacio){
	case HAY_HUECO_ASIGNABLE:
		t_segmento* hueco = list_get(tabla_huecos, huecoDisponible);
				char* pid = string_itoa(pedido->pid);
				tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, pid);

				u_int32_t limiteSegmento = hueco->base + pedido->tamanio;
				t_segmento* nuevoSegmento = crear_t_segmento(pedido->id_segmento, hueco->base, limiteSegmento);
				list_add(tabla_del_proceso, nuevoSegmento);
				log_info(logger, "Nuevo segmento: id %d, base %d, limite %d", nuevoSegmento->id, nuevoSegmento->base, nuevoSegmento->limite);

				if(hueco->limite == nuevoSegmento->limite){
					t_segmento* huecoOcupado = list_remove(tabla_huecos, huecoDisponible);
					log_info(logger, "Se ocupó completamente el hueco");
					free(huecoOcupado);

				}else
					hueco->base = nuevoSegmento->limite;

				estadoCreacion = CREACION_EXITOSA;
		break;
	case NO_HAY_HUECO_ASIGNABLE:
		log_error(logger, "No hay hueco libre disponible");
		estadoCreacion = OUT_OF_MEMORY;
		break;
	case HAY_ESPACIO_AL_COMPACTAR:
		//TODO: compactacion
		estadoCreacion = CREACION_EXITOSA;
		break;
	default:
		log_error(logger, "Espacio invalido");
	}
}


