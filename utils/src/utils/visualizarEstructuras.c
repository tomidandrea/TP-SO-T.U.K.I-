#include <utils/visualizarEstructuras.h>

void mostrarRegistros(t_registros* registros){
	printf("R AX: %s\n",registros->AX);
	printf("R BX: %s\n",registros->BX);
	printf("R CX: %s\n",registros->CX);
	printf("R DX: %s\n",registros->DX);
	printf("R EAX: %s\n",registros->EAX);
	printf("R EBX: %s\n",registros->EBX);
	printf("R ECX: %s\n",registros->ECX);
	printf("R EDX: %s\n",registros->EDX);
	printf("R RAX: %s\n",registros->RAX);
	printf("R RBX: %s\n",registros->RBX);
	printf("R RCX: %s\n",registros->RCX);
	printf("R RDX: %s\n",registros->RDX);
}


