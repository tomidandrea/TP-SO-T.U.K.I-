#include <consola.h>

void enviar_programa(t_list * instrucciones, int conexion) {

	int cant = list_size(instrucciones);                   // saco la cantidad de elementos de la lista para despues poder iterar rn el for hasta esa cantidad
	char* operacion; //= malloc(TAMANIO_OPERACION); no es necesario xq uso copiar()  // es el nombre de la instruccion (llamado operacion)
	int op_tamanio = 0;                                    // es el tamaño que ocupa la operacion
	int cant_parametros = 0;
	//t_instruccion* inst = malloc(sizeof(t_instruccion));
	// reservo espacio para la instruccion, no es necesario xq no vamos a crear una instruccion, solo obtenemos inst de la lista
	t_instruccion* inst;
	t_paquete *paquete = crear_paquete(PROGRAMA);         // creo paquete donde voy a meter todas las instrucciones en su buffer junto con sus tamanios


	for(int i = 0;i<cant;i++) {

	    inst = list_get(instrucciones,i);                  // cada instruccion la obtengo de la lista
	    operacion = copiar(inst->instruccion);
	    op_tamanio = strlen(operacion)+1;

	    agregar_a_paquete(paquete,operacion,op_tamanio);   // agrego al paquete la operacion y su tamaño

	    cant_parametros = cantParametros(operacion);       // saco la cantidad de parametros de la insruccion para poder iterar en el for que sigue
	    for(int i=0; i<cant_parametros; i++) {
	    	agregar_a_paquete(paquete,inst->parametros[i],strlen(inst->parametros[i])+1);   //agrego al paquete cada parametro de la instruccion con su tamaño
	    }
	    free(operacion);	//al copiar() la operacion estamos haciendo un malloc por loop
	}

	enviar_paquete(paquete,conexion);                // serializa el paquete y lo envia
	eliminar_paquete(paquete);                //elimina el paquete y lo que contiene

}




