#ifndef SRC_COMUNICACIONMEMORIA_H_
#define SRC_COMUNICACIONMEMORIA_H_

#include <utils/sockets.h>
#include <utils/general.h>
#include <utilsMemoria.h>
#include <semaphore.h>
#include <commons/string.h>

void escucharKernel();
void escucharCPU();
void escucharFS();

void enviarDiccionarioTablas(t_socket socket_kernel);

#endif
