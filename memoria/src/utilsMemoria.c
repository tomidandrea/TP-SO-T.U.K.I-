#include "utilsMemoria.h"
#define ENCONTRO_HUECO 1
#define NO_ENCONTRO_HUECO 0

extern void* espacioMemoria;
extern tabla_segmentos tabla_huecos;
extern t_log* logger;
extern t_config* config;
int cantidadMaxSegmentos;
t_dictionary* diccionarioTablas;
t_segmento* segmento0;
int huecoDisponible;
op_code estadoCreacion;
char* algoritmoConfig;

//TODO alejiti: ver si necesitamos mutex para el diccionario y tabla de huecos

t_segmento* crear_t_segmento(int id, u_int32_t base, u_int32_t limite){
	t_segmento* segmento = malloc(sizeof(t_segmento));
	segmento->id = id;
	segmento->base = base;
	segmento->limite = limite;
	return segmento;
}

void inicializarEstructuras(){
	//algoritmoConfig = config_get_string_value(config,"ALGORITMO_ASIGNACION");
	algoritmoConfig = "WORST";

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
	t_segmento* huecoAsignable;
	u_int32_t tamanio;
	u_int32_t tamanioTotal = 0; // acumula el tamaño de los huecos
	int cantidadHuecos = list_size(tabla_huecos);
	int condicion = NO_ENCONTRO_HUECO;

	log_debug(logger, "Llegó un pedido de segmento: Id: %d - Tamaño: %d", pedido->id_segmento, pedido->tamanio);

	//TODO alejiti: switch segun el algoritmo de asignacion en config
	switch (idAlgoritmo(algoritmoConfig)) {
		case FIRST_FIT:
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
		break;
		case BEST_FIT:
			condicion = NO_ENCONTRO_HUECO;
				for(int i=0;i<cantidadHuecos;i++){
					hueco = list_get(tabla_huecos, i);
					tamanio=obtenerTamanioSegmento(hueco);

						//log_debug(logger, "Hueco %d tiene tamaño: %d", i, tamanio);

						if(pedido->tamanio <= tamanio && condicion == NO_ENCONTRO_HUECO){ //si no encontro hueco guarda el primero en el que entra
							huecoAsignable = list_get(tabla_huecos,i);
							condicion = ENCONTRO_HUECO;
						}else if(pedido->tamanio <= tamanio && condicion == ENCONTRO_HUECO){ //si ya entro en algun hueco ahi los empieza a comparar
							huecoAsignable = obtenerHuecoSegunTamanio(huecoAsignable, hueco, BEST_FIT);
						}
						else{
							tamanioTotal += tamanio;
						}
				}
				 // guardo el hueco en una variable global
				if(condicion == ENCONTRO_HUECO){
					huecoDisponible = hueco->id;
					return HAY_HUECO_ASIGNABLE;

				}else if(tamanioTotal > pedido->tamanio){
					log_debug(logger, "Hay espacio si compactamos");
					return HAY_ESPACIO_AL_COMPACTAR;
				}else{
					return NO_HAY_HUECO_ASIGNABLE;
				}

			break;
		case WORST_FIT:
			condicion = NO_ENCONTRO_HUECO;
				for(int i=0;i<cantidadHuecos;i++){
					hueco = list_get(tabla_huecos, i);
					tamanio=obtenerTamanioSegmento(hueco);

						//log_debug(logger, "Hueco %d tiene tamaño: %d", i, tamanio);

						if(pedido->tamanio <= tamanio && condicion == NO_ENCONTRO_HUECO){ //si no encontro hueco guarda el primero en el que entra
							huecoAsignable = list_get(tabla_huecos,i);
							condicion = ENCONTRO_HUECO;
						}else if(pedido->tamanio <= tamanio && condicion == ENCONTRO_HUECO){ //si ya entro en algun hueco ahi los empieza a comparar
							huecoAsignable = obtenerHuecoSegunTamanio(huecoAsignable, hueco, WORST_FIT);
						}
						else{
							tamanioTotal += tamanio;
						}
				}
				 // guardo el hueco en una variable global
				if(condicion == ENCONTRO_HUECO){
					huecoDisponible = hueco->id;
					return HAY_HUECO_ASIGNABLE;

				}else if(tamanioTotal > pedido->tamanio){
					log_debug(logger, "Hay espacio si compactamos");
					return HAY_ESPACIO_AL_COMPACTAR;
				}else{
					return NO_HAY_HUECO_ASIGNABLE;
				}
			break;
	}

}

t_segmento* obtenerHuecoSegunTamanio(t_segmento* huecoAsignable, t_segmento* hueco, t_algoritmo_memoria algoritmo){
	int tamanioHuecoAsignable, tamanioHueco;
	tamanioHuecoAsignable = obtenerTamanioSegmento(huecoAsignable);
	tamanioHueco = obtenerTamanioSegmento(hueco);
	switch (algoritmo){
	case BEST_FIT:
		if(tamanioHuecoAsignable < tamanioHueco){ //si el tamanio a asignar es menor que el tamanio que tenia asignado, lo guarda
			return huecoAsignable;
		}else{
			return hueco;
		}
		break;
	case WORST_FIT:
		if(tamanioHuecoAsignable > tamanioHueco){ //si el tamanio a asignar es mayor que el tamanio que tenia asignado, lo guarda
			return huecoAsignable;
		}else{
			return hueco;
		}
	}


}

void crearSegmento(t_pedido_segmento* pedido) {
	int estadoEspacio = hayEspacio(pedido);
	t_segmento* nuevoSegmento;
	switch(estadoEspacio){
	case HAY_HUECO_ASIGNABLE:
		t_segmento* hueco = list_get(tabla_huecos, huecoDisponible);
				char* pid = string_itoa(pedido->pid);
				tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, pid);

				u_int32_t limiteSegmento = hueco->base + pedido->tamanio;
				nuevoSegmento = crear_t_segmento(pedido->id_segmento, hueco->base, limiteSegmento);
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
		//compactarTabla();
		estadoCreacion = CREACION_EXITOSA;
		break;
	default:
		log_error(logger, "Espacio invalido");
	}
}

void eliminarSegmento (t_pedido_segmento* pedido) {
	char* pid = string_itoa(pedido->pid);
	tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, pid);

	list_remove(tabla_del_proceso, pedido->id_segmento);
	log_info(logger, "Se removio completamente el segmento %d", pedido->id_segmento);

}

void enviarSegmentoCreado(t_socket socket_kernel, tabla_segmentos tabla_segmentos){
	t_paquete* paquete = crear_paquete(CREACION_EXITOSA);
	int ultimoElemento = list_size(tabla_segmentos) - 1;
	log_debug(logger, "Indice: %d", ultimoElemento);
	t_segmento* nuevoSegmento = list_get(tabla_segmentos, ultimoElemento);
	agregar_valor_estatico(paquete,&(nuevoSegmento->id));
	agregar_valor_uint(paquete,&(nuevoSegmento->base));
	agregar_valor_uint(paquete,&(nuevoSegmento->limite));

	enviar_paquete(paquete, socket_kernel);
	eliminar_paquete(paquete);
}
