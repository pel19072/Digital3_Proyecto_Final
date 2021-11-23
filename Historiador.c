/*    Nombre     : 	Lab8_Extras.c
      Autor      : 	Ricardo Pellecer Orellana
      Carné      :  19072
      Curso      :  Electrónica Digital 3
      Descripción: 	Programa para realizar un sistema de votación enter varios
                    dispositivos haciendo uso del protocolo de comunicación UDP
                    con broadcast.
*/

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
#include <fcntl.h>

#define MSG_SIZE 200	  // message size
#define INIT_VALUE	1		// Para el valor inicial del semáforo. Este valor
                        // asegura que solo una luz estará encedida al mismo
                        // tiempo, porque solo un hilo puede acceder a la región
                        // crítica a la vez.

// Función para mostrar los errores
void error(const char *msg){
    perror(msg);
    exit(0);
}

// Prototipos
void Manual_CMD(void *ptr); // Para envío manual de comandos

/**************************** Variables Globales ******************************/
// Semaphore
sem_t my_semaphore;
// UDP
int sockfd, n;
struct sockaddr_in server, addr, addr1, addr2;
char buffer_RX[MSG_SIZE], buffer_TX[MSG_SIZE];	// to store received messages or messages to be sent.
int boolval = 1;	// for a socket option
int length;



int main(int argc, char *argv[]){
  /******************************** Variables *********************************/
  // Interprocesos e interhilos
	int pipe_CtoPy;		// for file descriptors
  int dummy;        // Para creación de FIFOs en terminal
  // Hilos
  pthread_t threads[1];
	dummy = system("mkfifo CtoPy");
	/******************************* UDP Config *********************************/
	// Para determinar el puerto a usar
	if(argc != 2){
		printf("usage: %s port\n", argv[0]);
		exit(0);
	}
	// Creates socket. Connectionless
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		error("Opening socket");
	}
	server.sin_family = AF_INET;							// symbol constant for Internet domain
	server.sin_port = htons(atoi(argv[1]));		// port number
	server.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("192.168.1.255");

	addr.sin_family = AF_INET;							// symbol constant for Internet domain
	addr.sin_port = htons(atoi(argv[1]));		// port number
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

  addr1.sin_family = AF_INET;							// symbol constant for Internet domain
	addr1.sin_port = htons(atoi(argv[1]));		// port number
	addr1.sin_addr.s_addr = inet_addr("192.168.1.27"); //inet_addr("10.0.0.16");

  addr2.sin_family = AF_INET;							// symbol constant for Internet domain
	addr2.sin_port = htons(atoi(argv[1]));		// port number
	addr2.sin_addr.s_addr = inet_addr("192.168.1.27"); //inet_addr("10.0.0.9");

	length = sizeof(server);									// size of structure
	// binds the socket to the address of the host and the port number
	if(bind(sockfd, (struct sockaddr *)&server, length) < 0)
		error("Error binding socket.");
	// change socket permissions to allow broadcast
	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
   		error("Error setting socket options\n");
	/******************************* Threads ************************************/
  pthread_create(&threads[0], NULL, (void*)&Manual_CMD, NULL);
	/******************************* Named Pipes ********************************/
	if((pipe_CtoPy = open("CtoPy", O_WRONLY)) < 0){
		printf("pipe CtoPy error\n");
		exit(-1);
	}
	/******************************* Main Loop **********************************/
	while(1){
    // Aquí hay una recepción de datos a través de UDP con los RTUs y se guarda
    // en los buffers.
		/******************************* Recepción ********************************/
		memset(buffer_RX, 0, MSG_SIZE);	// sets all values to zero.
		// receive from a client
		n = recvfrom(sockfd, buffer_RX, MSG_SIZE, 0, (struct sockaddr *)&addr, &length);
    if(n < 0){
      error("recvfrom");
    }
		printf("%s", buffer_RX);
    fflush(stdout);
		/**************************** Envío por Pipe ******************************/
		if(write(pipe_CtoPy, buffer_RX, sizeof(buffer_RX)) != sizeof(buffer_RX))
		{
			printf("CtoPy pipe write error\n");
			exit(-1);
		}
	}
	return dummy;
}

void Manual_CMD(void *ptr){
  // Aquí hay un envío de datos a través de UDP con los RTUs y se guarda
  // en los buffers.
	/******************************** Variables *********************************/
	char cmd[MSG_SIZE] = "1.1";
	char decision[4] = "no\n";
	while(1){
		fflush(stdout);
		printf("Desea enviar un comando manual?(yes/no)");
		scanf("%s", decision);
		printf("Su decision fue %s\n", decision);
    fflush(stdout);
		if(strcmp(decision, "yes") == 0){
      fflush(stdout);
  		printf("A quien desea enviarlo?(1/2)");
  		scanf("%s", decision);
  		printf("Su decision fue %s\n", decision);
      fflush(stdout);
      if(strcmp(decision, "1") == 0){
        fflush(stdout);
    		printf("Desea Encender o Apagar?(1/0)");
    		scanf("%s", decision);
    		printf("Su decision fue %s\n", decision);
        fflush(stdout);
        if(strcmp(decision, "1") == 0){
          n = sendto(sockfd, "1.1", MSG_SIZE, 0,
    							(struct sockaddr *)&addr1, length);
    			if(n < 0){
    				error("sendto");
    			}
    			printf("se envio un dato\n");
          fflush(stdout);
        }
        else{
          n = sendto(sockfd, "1.0", MSG_SIZE, 0,
    							(struct sockaddr *)&addr1, length);
    			if(n < 0){
    				error("sendto");
    			}
    			printf("se envio un dato\n");
          fflush(stdout);
        }

      }
      else if(strcmp(decision, "2") == 0){
        fflush(stdout);
    		printf("Desea Encender o Apagar?(1/0)");
    		scanf("%s", decision);
    		printf("Su decision fue %s\n", decision);
        fflush(stdout);
        if(strcmp(decision, "1") == 0){
          n = sendto(sockfd, "0.1", MSG_SIZE, 0,
    							(struct sockaddr *)&addr1, length);
    			if(n < 0){
    				error("sendto");
    			}
    			printf("se envio un dato\n");
          fflush(stdout);
        }
        else{
          n = sendto(sockfd, "0.0", MSG_SIZE, 0,
    							(struct sockaddr *)&addr1, length);
    			if(n < 0){
    				error("sendto");
    			}
    			printf("se envio un dato\n");
          fflush(stdout);
        }

      }
		}
	}
	pthread_exit(0);
}
