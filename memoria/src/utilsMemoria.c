#include "utilsMemoria.h"

extern void* espacioMemoria;
tabla_segmentos* tablaSegmentos;
extern t_config* config;

void inicializarEstructuras(config){

	tablaSegmentos = create_list();
	t_segmento* segmento0;
	int tamanioMemoria = config_get_int_value(config,"TAM_MEMORIA");
	int tamanioSegmento0 = config_get_int_value(config,"TAM_SEGMENTO_0");
	int cantidadSegmentos = config_get_int_value(config,"CANT_SEGMENTOS");

	espacioMemoria = malloc(tamanioMemoria);
	segmento0->base = espacioMemoria;
	segmento0->id = 0;
	segmento0->limite = espacioMemoria + tamanioSegmento0;
	list_add(tablaSegmentos, segmento0);

}

void enviarSegmentosKernel(){

	t_paquete *paquete = crear_paquete(TABLA_SEGMENTOS_INICIAL);
	int cantidad = list_size(tablaSegmentos);
	t_segmento* segmento;
	for (int i = 0; i<cantidad; i++){
		segmento = list_get(tablaSegmentos, i);
		agregar_valor_estatico(paquete,segmento->base);
		agregar_valor_estatico(paquete,segmento->id);
		agregar_valor_estatico(paquete,segmento->limite);
	}

	//TODO Ver estas conxiones
	enviar_paquete(TABLA_SEGMENTOS_INICIAL, conexionMemoria);
	eliminar_paquete(TABLA_SEGMENTOS_INICIAL, conexionMemoria);
}


