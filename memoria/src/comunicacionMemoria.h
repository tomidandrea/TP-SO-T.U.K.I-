#ifndef SRC_COMUNICACIONMEMORIA_H_
#define SRC_COMUNICACIONMEMORIA_H_

#include <utils/general.h>
#include <utils/sockets.h>
#include <semaphore.h>

void escucharKernel();
void escucharCPU();
void escucharFS();
void inicializarEstructuras();
void enviarSegmentosKernel(t_socket socket_kernel);

#endif
