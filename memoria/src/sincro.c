#include<sincro.h>

// creamos los hilos

void crearEscucharKernel(){
	pthread_t hilo_kernel;
	pthread_create(&hilo_kernel,
					NULL,
					(void*) main,
					NULL);
	pthread_detach(hilo_kernel);
}
void crearEsucharCPU(){
	pthread_t hilo_CPU;
	pthread_create(&hilo_CPU,
					NULL,
					(void*) main,
					NULL);
	pthread_detach(hilo_CPU);
}
void crearEscucharFS(){
	pthread_t hilo_FS;
	pthread_create(&hilo_FS,
					NULL,
					(void*) main,
					NULL);
	pthread_detach(hilo_FS);
}
