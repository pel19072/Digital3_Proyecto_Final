/*    Nombre     : 	Historiador.c
      Autor      : 	Ricardo Pellecer Orellana
      Carné      :  19072
      Curso      :  Electrónica Digital 3
      Descripción: 	Programa para el historiador del sistema SCADA
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

#define MSG_SIZE 200	  // Tamaño del mensaje

// Función para mostrar los errores
void error(const char *msg){
    perror(msg);
    exit(0);
}

// Prototipos
void Manual_CMD(void *ptr); // Para envío manual de comandos

/**************************** Variables Globales ******************************/
// UDP
int sockfd, n;
struct sockaddr_in server, addr, addr1, addr2;
char buffer_RX[MSG_SIZE], buffer_TX[MSG_SIZE];	// to store received messages or messages to be sent.
int boolval = 1;	// for a socket option
int length;



int main(int argc, char *argv[]){
  /******************************** Variables *********************************/
  // Interprocesos e interhilos
	int pipe_CtoPy;		// File descriptor
  int dummy;        // Para creación de FIFOs en terminal
  // Hilos
  pthread_t threads[1];
  // Pipes
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
  /*
    * La estructura server fue utilizada para hacer el binding con el socket
    * La estructura addr fue utilizada para hacer la recepción de datos
    * La estructura addr1 fue utilizada para hacer el envío de datos al RTU 1
    * La estructura addr2 fue utilizada para hacer el envío de datos al RTU 2
    * Nótese que las IPs son hardcoded
  */
	server.sin_family = AF_INET;							// symbol constant for Internet domain
	server.sin_port = htons(atoi(argv[1]));		// port number
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

  addr1.sin_family = AF_INET;
	addr1.sin_port = htons(atoi(argv[1]));
	addr1.sin_addr.s_addr = inet_addr("10.0.0.21");

  addr2.sin_family = AF_INET;
	addr2.sin_port = htons(atoi(argv[1]));
	addr2.sin_addr.s_addr = inet_addr("10.0.0.22");

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
  /*
    * Creo un Pipe de escritura para mandar los datos recibido a Python, en
    donde se realiza la interpretación de los mismos para desplegarlos
  */
	if((pipe_CtoPy = open("CtoPy", O_WRONLY)) < 0){
		printf("pipe CtoPy error\n");
		exit(-1);
	}
	/******************************* Main Loop **********************************/
	while(1){
    /*
      * Aquí hay una recepción de datos a través de UDP con los RTUs y se guarda
      en el buffer_RX.
    */
		/******************************* Recepción ********************************/
		memset(buffer_RX, 0, MSG_SIZE);	// Limpia el buffer.
		// receive from a client
		n = recvfrom(sockfd, buffer_RX, MSG_SIZE, 0, (struct sockaddr *)&addr, &length);
    if(n < 0){
      error("recvfrom");
    }
    fflush(stdout);
		/**************************** Envío por Pipe ******************************/
    // Manda lo recibido por UDP a través del pipe a Python
		if(write(pipe_CtoPy, buffer_RX, sizeof(buffer_RX)) != sizeof(buffer_RX)){
			printf("CtoPy pipe write error\n");
			exit(-1);
		}
	}
	return dummy;
}

void Manual_CMD(void *ptr){
  /*
    * Aquí hay una lectura del pipe de Python a C que contiene el comando manual
    y el envío de dichos comandos manuales a través de UDP hacia los RTUs
  */
	/******************************** Variables *********************************/
	char cmd[6]; // Buffer que guarda los comandos manuales recibidos de Python
  int pipe_PytoC; // File descriptor
  int dummy;  // Para la creación del FIFO en terminal
  dummy = system("mkfifo PytoC");
  // Se abre el Pipe para leer únicamente
  if((pipe_PytoC = open("PytoC", O_RDONLY)) < 0){
		printf("pipe PytoC error\n");
		exit(-1);
	}
  // Se realiza la recepción de información a través del Pipe y se envía por UDP
	while(1){
    sleep(1);
    // Leo la información dada por el pipe y lo almaceno en cmd
    if(read(pipe_PytoC, &cmd, sizeof(cmd)) < 0){
			printf("PytoC pipe read error\n");
			exit(-1);
		}
    // Mando la información a ambos RTUs
    n = sendto(sockfd, cmd, 6, 0,
            (struct sockaddr *)&addr1, length);
    if(n < 0){
      error("sendto");
    }
    n = sendto(sockfd, cmd, 6, 0,
            (struct sockaddr *)&addr2, length);
    if(n < 0){
      error("sendto");
    }
	}
	pthread_exit(0);
}
