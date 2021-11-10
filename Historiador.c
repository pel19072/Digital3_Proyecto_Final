#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

#define INIT_VALUE	1		// Para el valor inicial del semáforo. Este valor
                        // asegura que solo una luz estará encedida al mismo
                        // tiempo, porque solo un hilo puede acceder a la región
                        // crítica a la vez.

// Prototipos
void Manual_CMD(void *ptr); // Para envío manual de comandos

sem_t my_semaphore;	// counting semaphore

int main(void)
{

  // Variables
  // Interprocesos
	char buffer[] = "Hola mundo";
	int pipe_CtoPy;		// for file descriptors
  int dummy;        // Para creación de FIFOs en terminal

  // Hilos
  pthread_t threads[1]; /*
                        * Creación de los hilos a usar para envío manual de
                        comandos
                        */

	dummy = system("mkfifo CtoPy"); 	// could be done separetely in each task,

  pthread_create(&threads[0], NULL, (void*)&My_thread1, NULL);

	if((pipe_CtoPy = open("CtoPy", O_WRONLY)) < 0)
	{
		printf("pipe CtoPy error\n");
		exit(-1);
	}
	while(1)
	{
    // Aquí hay una recepción de datos a través de UDP con los RTUs y se guarda
    // en los buffers.
		sleep(1);
		if(write(pipe_CtoPy, buffer, sizeof(buffer)) != sizeof(buffer))
		{
			printf("CtoPy pipe write error\n");
			exit(-1);
		}
	}
	return dummy;
}

void Manual_CMD(void *ptr)
{
  // Aquí hay un envío de datos a través de UDP con los RTUs y se guarda
  // en los buffers.
	pthread_exit(0);
}
