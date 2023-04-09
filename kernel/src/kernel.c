#include <kernel.h>
#include <utils/sockets.h>

int main(int argc, char* argv[]) {
    puts("Hello world!!");
    t_log* logger = iniciar_logger();

	t_socket server_fd = iniciar_servidor(logger);
	t_socket cliente_fd = esperar_cliente(server_fd, logger);

    return 0;
}

t_log* iniciar_logger(void)
{
	char* file = "kernel.log";
	char *process_name = "KERNEL";
	bool is_active_console = true;
	t_log_level level = LOG_LEVEL_DEBUG;
	t_log* nuevo_logger = log_create(file, process_name,is_active_console, level);

	return nuevo_logger;
}
