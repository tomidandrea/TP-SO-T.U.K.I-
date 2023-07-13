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

void* leer(u_int32_t direc, int tamanio, int pid) {
	//valor no tiene \0, tamaño es igual que el registro (4,8,16)
	void* valor = malloc(tamanio);
	memcpy(valor, espacioMemoria + direc, tamanio);
	log_info(logger,  "PID: %d- Acción: LEER - Dirección física: %d - Tamaño: %d- Origen: CPU", pid, direc, tamanio);

	return valor;
}

void escribir(u_int32_t direc, int tamanio, char* valor, int pid) {
	//valor tiene \0, tamaño es igual que el registro (4,8,16)
	memcpy(espacioMemoria + direc, valor, tamanio);
	log_info(logger,  "PID: %d- Acción: ESCRIBIR - Dirección física: %d - Tamaño: %d- Origen: CPU", pid, direc, tamanio);
}

u_int32_t obtenerTamanioSegmento(t_segmento* segmento){
	return segmento->limite - segmento->base;
}

EstadoMemoria asignacion_first(int tamanioPedido){
	t_segmento* hueco;
	u_int32_t tamanio;
	u_int32_t tamanioTotal = 0; // acumula el tamaño de los huecos
	int cantidadHuecos = list_size(tabla_huecos);
	for(int i=0;i<cantidadHuecos;i++){
		hueco = list_get(tabla_huecos, i);
		tamanio=obtenerTamanioSegmento(hueco);
		//log_debug(logger, "Hueco %d - Tamaño: %d", hueco->id, tamanio);
		if(tamanioPedido <= tamanio){
			//log_info(logger, "Hueco libre disponible");
			huecoDisponible = hueco->id; // guardo el hueco en una variable global
			log_debug(logger,"Hueco asignable: %d - Tamaño: %d", huecoDisponible, tamanio);
			return HAY_HUECO_ASIGNABLE;
		}else{
			tamanioTotal += tamanio;
		}
	}
	if(tamanioTotal >= tamanioPedido){
		log_debug(logger, "Hay espacio si compactamos");
		return HAY_ESPACIO_AL_COMPACTAR;
	}
	return NO_HAY_HUECO_ASIGNABLE;
}

EstadoMemoria asignacion_por_algoritmo(int tamanioPedido, int algoritmo){
	t_segmento* hueco;
	t_segmento* huecoAsignable;
	u_int32_t tamanio;
	u_int32_t tamanioTotal = 0; // acumula el tamaño de los huecos
	int cantidadHuecos = list_size(tabla_huecos);
	int condicion = NO_ENCONTRO_HUECO;
	for(int i=0;i<cantidadHuecos;i++){
		hueco = list_get(tabla_huecos, i);
		tamanio=obtenerTamanioSegmento(hueco);

		//log_debug(logger, "Hueco %d - Tamaño: %d", i, tamanio);

		if(tamanioPedido <= tamanio && condicion == NO_ENCONTRO_HUECO){
			//si no encontro hueco guarda el primero en el que entra
			huecoAsignable = hueco;
			condicion = ENCONTRO_HUECO;
		}else if(tamanioPedido <= tamanio && condicion == ENCONTRO_HUECO){
			//si ya entro en algun hueco ahi los empieza a comparar
			huecoAsignable = obtenerHuecoSegunAlgoritmo(huecoAsignable, hueco, algoritmo);
		}
		else{
			tamanioTotal += tamanio;
		}
	}
	 // guardo el hueco en una variable global
	if(condicion == ENCONTRO_HUECO){
		huecoDisponible = huecoAsignable->id;
		mostrarTablaHuecos(tabla_huecos);
		log_debug(logger,"Hueco asignable: %d - Tamaño: %d", huecoDisponible, obtenerTamanioSegmento(huecoAsignable));
		return HAY_HUECO_ASIGNABLE;

	}else if(tamanioTotal >= tamanioPedido){
		log_debug(logger, "Hay espacio si compactamos");
		return HAY_ESPACIO_AL_COMPACTAR;
	}else{
		return NO_HAY_HUECO_ASIGNABLE;
	}
}


// Dado un pedido de creación de segmento y el algoritmo de asignación,
// retorna si hay hueco asignable y guarda el mismo en una variable global.
// Si no entra el pedido en ningún hueco, pero si en la suma de los mismos,
// retorna que debe compactarse.
int hayEspacio(t_pedido_segmento* pedido){
	/*t_segmento* hueco;
	t_segmento* huecoAsignable;
	u_int32_t tamanio;
	u_int32_t tamanioTotal = 0; // acumula el tamaño de los huecos
	int cantidadHuecos = list_size(tabla_huecos);
	int condicion = NO_ENCONTRO_HUECO;*/
	EstadoMemoria estado;
	log_debug(logger, "Pedido: id %d, tamanio %d", pedido->id_segmento, pedido->tamanio);
	switch (idAlgoritmo(algoritmoConfig)) {
		case FIRST_FIT:
			estado = asignacion_first(pedido->tamanio);
			break;
		case BEST_FIT:
			estado = asignacion_por_algoritmo(pedido->tamanio, BEST_FIT);
			break;
		case WORST_FIT:
			estado = asignacion_por_algoritmo(pedido->tamanio, WORST_FIT);
			break;
		default:
			log_error(logger, "Error al reconocer algoritmo de asignación");
			return -1;
	}
	return estado;

}

t_segmento* obtenerHuecoSegunAlgoritmo(t_segmento* huecoAsignable, t_segmento* hueco, t_algoritmo_memoria algoritmo){
	int tamanioHuecoAsignable, tamanioHueco;
	tamanioHuecoAsignable = obtenerTamanioSegmento(huecoAsignable);
	tamanioHueco = obtenerTamanioSegmento(hueco);
	switch (algoritmo){
	case BEST_FIT:
		if(tamanioHuecoAsignable <= tamanioHueco){ //si el tamanio a asignar es menor que el tamanio que tenia asignado, lo guarda
			return huecoAsignable;
		}else{
			return hueco;
		}
		break;
	case WORST_FIT:
		if(tamanioHuecoAsignable >= tamanioHueco){ //si el tamanio a asignar es mayor que el tamanio que tenia asignado, lo guarda
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

op_code crearSegmento(t_pedido_segmento* pedido) {
	int estadoEspacio = hayEspacio(pedido);
	t_segmento* nuevoSegmento;
	op_code estadoCreacion;
	switch(estadoEspacio){
	case HAY_HUECO_ASIGNABLE:
		t_segmento* hueco = obtenerHuecoPorId(tabla_huecos, huecoDisponible);
				char* pid = string_itoa(pedido->pid);
				tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, pid);
				/*printf("-- Tabla de segmentos del proceso antes --\n");
				mostrarListaSegmentos(tabla_del_proceso);*/

				u_int32_t limiteSegmento = hueco->base + pedido->tamanio;
				nuevoSegmento = crear_t_segmento(pedido->id_segmento, hueco->base, limiteSegmento);
				list_add(tabla_del_proceso, nuevoSegmento);

				/*printf("-- Tabla de segmentos del proceso despues --\n");
				mostrarListaSegmentos(tabla_del_proceso);*/

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
	return estadoCreacion;
}

tabla_segmentos unificarTablas(){
	tabla_segmentos segmentosGlobales = list_create();

	int cantidadDeTablas = dictionary_size(diccionarioTablas);
	t_list* lista_pid = dictionary_keys(diccionarioTablas);

	for(int i = 0; i < cantidadDeTablas;i++){
		char* valor = list_get(lista_pid, i);
		tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, valor);
		printf("-- Tabla de segmentos del proceso que agrego a global --\n");
		mostrarListaSegmentos(tabla_del_proceso);
		//list_add_all(segmentosGlobales, tabla_del_proceso);
		int cantidadSegmentos = list_size(tabla_del_proceso);
		for (int j = 0; j < cantidadSegmentos; ++j) {
			t_segmento* seg = list_get(tabla_del_proceso, j);
			if(seg->id != 0 || i==0){
				list_add_sorted(segmentosGlobales, seg, esMenorBase);
			}
		}
		//todo ver como limpiar esto
		//list_clean(tabla_del_proceso);
	}
	printf("-- Tabla de segmentos globales --\n");
	mostrarListaSegmentos(segmentosGlobales);

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
	//list_sort(tablaSegmentosGlobales, esMenorBase); ordeno dentro de unificar tablas
	/*for(int i = 0; i<tamanioLista;i++){
		t_segmento* seg = list_get(tablaSegmentosGlobales,i);
		printf("Seg %d: base %d, limite %d\n", seg->id, seg->base,seg->limite);
	}*/
	t_segmento* segmentoActual = list_get(tablaSegmentosGlobales,0);
	for(int i = 1; i<tamanioLista;i++){
		t_segmento* segmentoSiguiente = list_get(tablaSegmentosGlobales,i);
		if(segmentoActual->limite != segmentoSiguiente->base){
			reubicarEspacioDeMemoria(segmentoSiguiente, segmentoActual->limite);
		}
		segmentoActual = segmentoSiguiente;
	}

	t_segmento* hueco = crear_t_segmento(++huecoId, segmentoActual->limite, tamanioMemoria);
	log_debug(logger, "Nuevo hueco %d: con base %d y limite %d", hueco->id, hueco->base, hueco->limite);
	list_clean_and_destroy_elements(tabla_huecos, free);
	list_add(tabla_huecos, hueco);

}

void agregarHueco(t_segmento* segmento){
	bool encontrado = false;
	int cantidadHuecos = list_size(tabla_huecos);
	t_segmento* huecoSuperior; //guardo si hay un hueco superior para fijarme si hay uno inferior, unificar desde base del superior hasta limite del inferior
		for (int i = 0; i < cantidadHuecos; i++) {
			t_segmento* hueco = list_get(tabla_huecos, i);
			if(segmento->limite < hueco->base){
				//si el hueco esta por debajo del segmento no hace falta seguir buscando
				break;
			}else if(segmento->limite == hueco->base){
				//hueco pegado abajo
				if(encontrado){ //significa que encontro un hueco pegado arriba en la iteracion anterior
					huecoSuperior->limite = hueco->limite;
					list_remove_and_destroy_element(tabla_huecos, i, free);
					log_debug(logger, "Unifico hueco Superior e Inferior: %d - Base: %d Limite: %d", huecoSuperior->id, huecoSuperior->base, huecoSuperior->limite);
				}else{
					hueco->base = segmento->base;
					encontrado = true;
					log_debug(logger, "Unifico hueco Inferior: %d - Base: %d Limite: %d", hueco->id, hueco->base, hueco->limite);
				}
				break;
			}else if(segmento->base == hueco->limite){
				//hueco pegado arriba
				huecoSuperior = list_get(tabla_huecos, i);
				huecoSuperior->limite = segmento->limite;
				encontrado = true;
				log_debug(logger, "Unifico hueco Superior: %d - Base: %d Limite: %d", hueco->id, hueco->base, hueco->limite);
			}
		}
		if(!encontrado){
			t_segmento* hueco = crear_t_segmento(++huecoId, segmento->base, segmento->limite);
			list_add_sorted(tabla_huecos, hueco, esMenorBase);
			log_debug(logger, "Nuevo hueco: %d - Base: %d Limite: %d", hueco->id, hueco->base, hueco->limite);
		}
}

void eliminarSegmento (t_pedido_segmento* pedido) {
	char* pid = string_itoa(pedido->pid);
	tabla_segmentos tabla_del_proceso = dictionary_get(diccionarioTablas, pid);
	t_segmento* segmento; //= malloc(sizeof(t_segmento)); //creo que no es necesario el malloc

	int indice = obtenerIndiceSegmento(tabla_del_proceso, pedido->id_segmento);
	segmento = list_remove(tabla_del_proceso, indice);

	//int cantidadHuecos = list_size(tabla_huecos);
	agregarHueco(segmento);

	//t_segmento* hueco = crear_t_segmento(++huecoId, segmento->base, segmento->limite);
	//list_add_sorted(tabla_huecos, hueco, esMenorBase);

	log_info(logger, "PID: %s - Eliminar Segmento: %d - Base: %d - TAMAÑO: %d", pid, segmento->id, segmento->base, segmento->limite-segmento->base);

	//free(pid);
	//free(segmento);

}

void enviarSegmentoCreado(t_socket socket_kernel, tabla_segmentos tabla_segmentos){
	t_paquete* paquete = crear_paquete(CREACION_EXITOSA);
	int ultimoElemento = list_size(tabla_segmentos) - 1;
	t_segmento* nuevoSegmento = list_get(tabla_segmentos, ultimoElemento);
	agregar_valor_estatico(paquete,&(nuevoSegmento->id));
	agregar_valor_uint(paquete,&(nuevoSegmento->base));
	agregar_valor_uint(paquete,&(nuevoSegmento->limite));

	enviar_paquete(paquete, socket_kernel);
	//printf("cod: %d\n", paquete->codigo_operacion);
	eliminar_paquete(paquete);
	mostrarListaSegmentos(tabla_segmentos);

	log_info(logger, "--- Segmento enviado a kernel ---");
}

void actualizarHuecos(tabla_segmentos tablaProceso){
	//int cantidadHuecos = list_size(tabla_huecos);
	t_segmento* segmento;
	int cantidadSegmentos = list_size(tablaProceso);

	for(int t = 0; t<cantidadSegmentos;t++){
			segmento = list_get(tablaProceso, t);
			agregarHueco(segmento);
	}
	mostrarTablaHuecos(tabla_huecos);
}

void liberarEstructurasProceso(char* pid){
	tabla_segmentos tablaProceso;
	tablaProceso = dictionary_remove(diccionarioTablas, pid);
	removerSegmento0(tablaProceso);
	actualizarHuecos(tablaProceso);

	list_destroy_and_destroy_elements(tablaProceso, free);
	//list_destroy(tablaProceso);
}



void liberar_memoria(){
	log_destroy(logger);
	config_destroy(config);
	dictionary_clean_and_destroy_elements(diccionarioTablas, liberarTablaSegmentos);
	list_destroy_and_destroy_elements(tabla_huecos, free);
	free(segmento0);
	free(espacioMemoria);
}
