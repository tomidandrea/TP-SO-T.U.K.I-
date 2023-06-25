#include "utilsMemoria.h"
#define ENCONTRO_HUECO 1
#define NO_ENCONTRO_HUECO 0

extern void* espacioMemoria;
extern tabla_segmentos tabla_huecos;
extern t_log* logger;
extern t_config* config;
int cantidadMaxSegmentos;
int tamanioMemoria;
t_dictionary* diccionarioTablas;
t_segmento* segmento0;
int huecoDisponible;
int huecoId;
op_code estadoCreacion;
char* algoritmoConfig;

//TODO alejiti: ver si necesitamos mutex para el diccionario y tabla de huecos

// ---- INICIO DE MEMORIA ----

t_segmento* crear_t_segmento(int id, u_int32_t base, u_int32_t limite){
	t_segmento* segmento = malloc(sizeof(t_segmento));
	segmento->id = id;
	segmento->base = base;
	segmento->limite = limite;
	return segmento;
}

void inicializarEstructuras(){
	algoritmoConfig = config_get_string_value(config,"ALGORITMO_ASIGNACION");
	//algoritmoConfig = "BEST";

	diccionarioTablas = dictionary_create();

	tamanioMemoria = config_get_int_value(config,"TAM_MEMORIA");
	int tamanioSegmento0 = config_get_int_value(config,"TAM_SEGMENTO_0");
	cantidadMaxSegmentos = config_get_int_value(config,"CANT_SEGMENTOS");

	espacioMemoria = malloc(tamanioMemoria);

	segmento0 = crear_t_segmento(0, 0, tamanioSegmento0);
	huecoId = segmento0->id;

	tabla_huecos = inicializarTablaHuecosLibres(tamanioMemoria,tamanioSegmento0) ;
}

tabla_segmentos inicializarTablaHuecosLibres(int tamanioMemoria,int tamanioSegmento0){
	//iniciamos la tabla con un hueco libre, a partir de donde termina el seg0 hasta el tamaño de la memoria
	t_segmento* hueco = crear_t_segmento(0, tamanioSegmento0, tamanioMemoria);
	tabla_segmentos tabla_segmentos = list_create();
	list_add(tabla_segmentos, hueco);
	return tabla_segmentos;
}

// ----- SEGMENTACIÓN -----
//TODO: eliminar
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

void enviarSegmentoCreado(t_socket socket_kernel, tabla_segmentos tabla_segmentos){
	t_paquete* paquete = crear_paquete(CREACION_EXITOSA);
	int ultimoElemento = list_size(tabla_segmentos) - 1;
	t_segmento* nuevoSegmento = list_get(tabla_segmentos, ultimoElemento);
	agregar_valor_estatico(paquete,&(nuevoSegmento->id));
	agregar_valor_uint(paquete,&(nuevoSegmento->base));
	agregar_valor_uint(paquete,&(nuevoSegmento->limite));

	enviar_paquete(paquete, socket_kernel);
	eliminar_paquete(paquete);

	log_info(logger, "--- Segmento enviado a kernel ---");
}

u_int32_t obtenerTamanioSegmento(t_segmento* segmento){
	return segmento->limite - segmento->base;
}

// Dado un pedido de creación de segmento y el algoritmo de asignación,
// retorna si hay hueco asignable y guarda el mismo en una variable global.
// Si no entra el pedido en ningún hueco, pero si en la suma de los mismos,
// retorna que debe compactarse.
int hayEspacio(t_pedido_segmento* pedido){
	t_segmento* hueco;
	t_segmento* huecoAsignable;
	u_int32_t tamanio;
	u_int32_t tamanioTotal = 0; // acumula el tamaño de los huecos
	int cantidadHuecos = list_size(tabla_huecos);
	int condicion = NO_ENCONTRO_HUECO;
	log_debug(logger, "Pedido: id %d, tamanio %d", pedido->id_segmento, pedido->tamanio);
	switch (idAlgoritmo(algoritmoConfig)) {
		case FIRST_FIT:
			for(int i=0;i<cantidadHuecos;i++){
				hueco = list_get(tabla_huecos, i);
				tamanio=obtenerTamanioSegmento(hueco);
				log_debug(logger, "Hueco %d - Tamaño: %d", hueco->id, tamanio);
				if(pedido->tamanio <= tamanio){
					//log_info(logger, "Hueco libre disponible");
					huecoDisponible = hueco->id; // guardo el hueco en una variable global
					log_debug(logger,"Hueco asignable: %d - Tamaño: %d", huecoDisponible, tamanio);
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

					log_debug(logger, "Hueco %d - Tamaño: %d", i, tamanio);

					if(pedido->tamanio <= tamanio && condicion == NO_ENCONTRO_HUECO){ //si no encontro hueco guarda el primero en el que entra
						huecoAsignable = hueco;
						condicion = ENCONTRO_HUECO;
					}else if(pedido->tamanio <= tamanio && condicion == ENCONTRO_HUECO){ //si ya entro en algun hueco ahi los empieza a comparar
						huecoAsignable = obtenerHuecoSegunAlgoritmo(huecoAsignable, hueco, BEST_FIT);
					}
					else{
						tamanioTotal += tamanio;
					}
				}
				 // guardo el hueco en una variable global
				if(condicion == ENCONTRO_HUECO){
					huecoDisponible = huecoAsignable->id;
					log_debug(logger,"Hueco asignable: %d - Tamaño: %d", huecoDisponible, obtenerTamanioSegmento(huecoAsignable));
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

						log_debug(logger, "Hueco %d - Tamaño: %d", i, tamanio);

						if(pedido->tamanio <= tamanio && condicion == NO_ENCONTRO_HUECO){ //si no encontro hueco guarda el primero en el que entra
							huecoAsignable = list_get(tabla_huecos,i);
							condicion = ENCONTRO_HUECO;
						}else if(pedido->tamanio <= tamanio && condicion == ENCONTRO_HUECO){ //si ya entro en algun hueco ahi los empieza a comparar
							huecoAsignable = obtenerHuecoSegunAlgoritmo(huecoAsignable, hueco, WORST_FIT);
						}
						else{
							tamanioTotal += tamanio;
						}
				}
				 // guardo el hueco en una variable global
				if(condicion == ENCONTRO_HUECO){
					huecoDisponible = huecoAsignable->id;
					log_debug(logger,"Hueco asignable: %d - Tamaño: %d", huecoDisponible, obtenerTamanioSegmento(huecoAsignable));
					return HAY_HUECO_ASIGNABLE;

				}else if(tamanioTotal > pedido->tamanio){
					log_debug(logger, "Hay espacio si compactamos");
					return HAY_ESPACIO_AL_COMPACTAR;

				}else{
					return NO_HAY_HUECO_ASIGNABLE;
				}
			break;
		default:
			log_error(logger, "Error al reconocer algoritmo de asignación");
			return -1;
	}

}

t_segmento* obtenerHuecoSegunAlgoritmo(t_segmento* huecoAsignable, t_segmento* hueco, t_algoritmo_memoria algoritmo){
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
	default:
			log_error(logger, "Error al obtener algoritmo de asignación");
			exit(EXIT_FAILURE);
	}
}

t_segmento* obtenerHuecoPorId(tabla_segmentos tabla_huecos, int huecoDisponible){
	t_segmento* hueco;
	int tamanio = list_size(tabla_huecos);
	for(int i=0; i<tamanio;i++){
		hueco = list_get(tabla_huecos, i);
		if(hueco->id == huecoDisponible)
			return hueco;
	}
	return NULL;
}

void removerHuecoPorId(tabla_segmentos tabla_huecos, int huecoDisponible){
	t_segmento* hueco;
	int tamanio = list_size(tabla_huecos);
	for(int i=0; i<tamanio;i++){
		hueco = list_get(tabla_huecos, i);
		if(hueco->id == huecoDisponible){
			log_debug(logger, "Libero hueco con id:%d", huecoDisponible);
			hueco = list_remove(tabla_huecos,i);
			free(hueco);
			break;
		}
	}
}

void crearSegmento(t_pedido_segmento* pedido) {
	int estadoEspacio = hayEspacio(pedido);
	t_segmento* nuevoSegmento;
	switch(estadoEspacio){
	case HAY_HUECO_ASIGNABLE:
		t_segmento* hueco = obtenerHuecoPorId(tabla_huecos, huecoDisponible);
				char* pid = string_itoa(pedido->pid);
				tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, pid);

				u_int32_t limiteSegmento = hueco->base + pedido->tamanio;
				nuevoSegmento = crear_t_segmento(pedido->id_segmento, hueco->base, limiteSegmento);
				list_add(tabla_del_proceso, nuevoSegmento);

				log_info(logger, "PID: %s - Crear Segmento: %d - Base: %d - TAMAÑO: %d", pid, nuevoSegmento->id, nuevoSegmento->base, nuevoSegmento->limite - nuevoSegmento->base);

				if(hueco->limite == nuevoSegmento->limite){
					removerHuecoPorId(tabla_huecos, huecoDisponible);
					log_info(logger, "Se ocupó completamente el hueco");
				}else
					hueco->base = nuevoSegmento->limite;

				estadoCreacion = CREACION_EXITOSA;
				free(pid);
		break;
	case NO_HAY_HUECO_ASIGNABLE:
		log_error(logger, "No hay hueco libre disponible");
		estadoCreacion = OUT_OF_MEMORY;
		break;
	case HAY_ESPACIO_AL_COMPACTAR:
		estadoCreacion = PEDIDO_COMPACTAR;
		break;
	default:
		log_error(logger, "Espacio invalido");
	}
}

tabla_segmentos unificarTablas(){
	tabla_segmentos segmentosGlobales = list_create();
	int cantidadDeTablas = dictionary_size(diccionarioTablas);


	for(int i = 1;i<=cantidadDeTablas;i++){
		char* valor = string_itoa(i);
		tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, valor);
		list_add_all(segmentosGlobales, tabla_del_proceso);
		//todo ver como limpiar esto
		//list_clean(tabla_del_proceso);
	}
	return segmentosGlobales;
}

bool esMenorBase(void* seg1, void* seg2){
	if(((t_segmento*)seg1)->base < ((t_segmento*)seg2)->base){
		return true;
	}
	else
		return false;
}

void reubicarEspacioDeMemoria(t_segmento* segmento, u_int32_t limite){
	log_info(logger, "Reubico segmento %d: de la base %d a la base %d", segmento->id, segmento->base, limite);
	int tamanio = 	segmento->limite - segmento->base;
	void* contenido = malloc(tamanio);
	memcpy(contenido, espacioMemoria + segmento->base, tamanio);

	segmento->base = limite;
	segmento->limite = segmento->base + tamanio;

	memcpy(espacioMemoria + segmento->base, contenido, tamanio); //asigno espacio de memoria con la "Nueva base"
}

void compactar(t_pedido_segmento* pedido){
	log_info(logger,"entre en compactar()");
	tabla_segmentos tablaSegmentosGlobales = unificarTablas();
	int tamanioLista = list_size(tablaSegmentosGlobales);
	list_sort(tablaSegmentosGlobales, esMenorBase);
	/*for(int i = 0; i<tamanioLista;i++){
		t_segmento* seg = list_get(tablaSegmentosGlobales,i);
		printf("Seg %d: base %d, limite %d\n", seg->id, seg->base,seg->limite);
	}*/
	t_segmento* segmentoActual = list_get(tablaSegmentosGlobales,0);
	for(int i = 1; i<tamanioLista;i++){
		t_segmento* segmentoSiguiente = list_get(tablaSegmentosGlobales,i);
		if(segmentoActual->limite != segmentoSiguiente->base){
			reubicarEspacioDeMemoria(segmentoSiguiente, segmentoActual->limite);
			printf("Diferente base alejo se la come\n");
		}
		segmentoActual = segmentoSiguiente;
	}

	//TODO: crear una funcion a parte para la limpieza de la tabla de huecos
	t_segmento* hueco = crear_t_segmento(++huecoId, segmentoActual->limite, tamanioMemoria);
	log_debug(logger, "Nuevo hueco %d: con base %d y limite %d", hueco->id, hueco->base, hueco->limite);
	list_clean_and_destroy_elements(tabla_huecos, free);
	list_add(tabla_huecos, hueco);

	//TODO: hacer una funcion a parte para no llamar a crearSegmento
	crearSegmento(pedido);

	/*for(int i = 0; i<tamanioLista;i++){
		t_segmento* seg = list_get(tablaSegmentosGlobales,i);
		printf("Seg %d: base %d, limite %d\n", seg->id, seg->base,seg->limite);
	}*/
}

int obtenerIndiceSegmento(tabla_segmentos tabla, int id_segmento) {
	t_segmento* seg;
	int indice = -1;
	for(int k=0; k<list_size(tabla);k++){
		seg = list_get(tabla, k);
		if(seg->id == id_segmento){
			indice = k;
		}
	}
	return indice;
}

void eliminarSegmento (t_pedido_segmento* pedido) {
	char* pid = string_itoa(pedido->pid);
	tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, pid);
	t_segmento* segmento = malloc(sizeof(t_segmento));

	int indice = obtenerIndiceSegmento(tabla_del_proceso, pedido->id_segmento);
	segmento = list_remove(tabla_del_proceso, indice);

	//int cantidadHuecos = list_size(tabla_huecos);

	t_segmento* hueco = crear_t_segmento(++huecoId, segmento->base, segmento->limite);
	list_add_sorted(tabla_huecos, hueco, esMenorBase);
	log_info(logger, "PID: %s - Eliminar Segmento: %d - Base: %d - TAMAÑO: %d", pid, segmento->id, segmento->base, segmento->limite-segmento->base);

	//free(pid);
	//free(segmento);

}

// ----- FIN DE PROCESO -----

void actualizarHuecos(tabla_segmentos tablaProceso){
	//int cantidadHuecos = list_size(tabla_huecos);
	t_segmento* segmento;
	int cantidadSegmentos = list_size(tablaProceso);

	for(int t = 1; t<cantidadSegmentos;t++){
			segmento = list_get(tablaProceso, t);

			segmento->id = ++huecoId;
			list_add_sorted(tabla_huecos, segmento, esMenorBase);
			log_debug(logger, "Nuevo hueco: %d - Base: %d Limite: %d", segmento->id, segmento->base, segmento->limite);
	}
}

void liberarEstructurasProceso(char* pid){
	tabla_segmentos tablaProceso;
	tablaProceso = dictionary_remove(diccionarioTablas, pid);

	actualizarHuecos(tablaProceso);

	list_destroy_and_destroy_elements(tablaProceso, free);
	//list_destroy(tablaProceso);
}
