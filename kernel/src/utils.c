#include <utils.h>

extern t_config* config;
extern t_log* logger;

int processID=0;

t_pcb* crearPCB(t_list* listaInstrucciones){

	t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->pid = ++processID;
    pcb->pc = 0;
    pcb->instrucciones = list_duplicate(listaInstrucciones);
    pcb->estado = NEW;

    return pcb;
}

void mandar_pcb_a_CPU(t_pcb* proceso){
	char* ip = config_get_string_value(config,"IP_CPU");
	char* puerto = config_get_string_value(config,"PUERTO_CPU");
	t_socket conexion = crear_conexion(ip, puerto, logger);

	// esto implementarlo para lo nuestro (copiado de consola)

	char* operacion;                                       // es el nombre de la instruccion (llamado operacion)
	t_paquete *paquete = crear_paquete();                  // creo paquete donde voy a meter todas las instrucciones en su buffer junto con sus tamanios
	int op_tamanio = 0;
	t_instruccion* inst = malloc(sizeof(t_instruccion));   // reservo espacio para la instruccion
	int cant = list_size(proceso->instrucciones);
	int cant_parametros = 0;

	agregar_a_paquete(proceso -> pid,operacion,sizeof(proceso -> pid));
	agregar_a_paquete(proceso -> pc,operacion,sizeof(proceso -> pc));

	for(int i = 0;i<cant;i++) {

	    inst = list_get(proceso -> instrucciones,i);                  // cada instruccion la obtengo de la lista
	    operacion = string_duplicate(inst -> instruccion);
	    op_tamanio = strlen(operacion)+1;

	    agregar_a_paquete(paquete,operacion,op_tamanio);   // agrego al paquete la operacion y su tamaño

	    cant_parametros = cantParametros(operacion);       // saco la cantidad de parametros de la insruccion para poder iterar en el for que sigue
	    for(int i=0; i<cant_parametros; i++) {
	    	agregar_a_paquete(paquete,inst->parametros[i],strlen(inst->parametros[i])+1);   //agrego al paquete cada parametro de la instruccion con su tamaño
	    }

	}

	enviar_paquete(paquete,conexion);                // serializa el paquete y lo envia

	eliminar_paquete(paquete);                //elimina el paquete y todo lo que contiene



}

