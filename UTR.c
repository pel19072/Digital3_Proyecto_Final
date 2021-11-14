
//***************************Librerias***************************************
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <wiringPiSPI.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>
#include <sys/time.h>

#define SPI_CHANNEL      0 // Canal SPI de la Raspberry Pi, 0 ó 1
#define SPI_SPEED  1500000 // Velocidad de la comunicación SPI (reloj, en HZ)
                            // Máxima de 3.6 MHz con VDD = 5V, 1.2 MHz con VDD = 2.7V
#define ADC_CHANNEL       0 // Canal A/D del MCP3002 a usar, 0 ó 1
#define  MSG_SIZE 80
//**************************Variables globales*****************************
int debounce, debounce1, var, b1, b2, sw1, sw2 = 0;
int UTR_identifier = 1;
uint16_t ADCvalue;
unsigned int length;
int sockfd, n,  perder = 0;
char buffer[MSG_SIZE];
char broad_ip[10];
char* IP_adress;
char copia_IP[1];
char* tok;
int save_ip, save_ip1, save_ip2, save_ip3 = 0;
int boolval = 1;			// for a socket option
int revision[3];
char reporte[MSG_SIZE];
struct sockaddr_in server, addr;// to store received messages or messages to be sent.
//*******************prototipos de funciones*********************************
void handlerb1 (void);
void handlerb2 (void);
void handlers1 (void);
void handlers2 (void);
uint16_t get_ADC(int channel);
void alarma (void *ptr);
void PWM (int pin, float del_alto, float del_bajo);
void PWM_off (int pin);
void error(const char *msg);
void ADC_read (void *ptr);
void recibir (void *ptr);
//************************************Hilos***********************************

//**********************************Main*************************************
int main(int argc, char *argv[]) {
    pthread_t thread2,thread3, thread4;//identificador de los hilos
    //FILE *fp_final;
   // char ADC[4];
    //char cadena[1000][5];
	wiringPiSetupGpio(); //funcion de configuracion para usar el pinout normal
	pinMode(26,INPUT); //pin para el boton como entrada
    pinMode(20,INPUT); //pin para el boton como entrada
    pinMode(21,INPUT); //pin para el boton como entrada
    pinMode(19,INPUT); //pin para el boton como entrada
    pinMode(18,OUTPUT); //pin para el boton como entrada
    pinMode(5,OUTPUT); //pin para el boton como entrada
    pinMode(6,OUTPUT); //pin para el boton como entrada
	pullUpDnControl(26,PUD_DOWN); //pin del boton configurado en pulldown
    pullUpDnControl(20,PUD_DOWN); //pin del boton configurado en pulldown
    pullUpDnControl(21,PUD_DOWN); //pin del boton configurado en pulldown
    pullUpDnControl(19,PUD_DOWN); //pin del boton configurado en pulldown
    wiringPiISR (26, INT_EDGE_BOTH,  (void*) &handlerb1);
    wiringPiISR (19, INT_EDGE_BOTH,  (void*) &handlerb2);
    wiringPiISR (20, INT_EDGE_BOTH,  (void*) &handlers1);
    wiringPiISR (21, INT_EDGE_BOTH,  (void*) &handlers2);
    if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) < 0) {
    printf("wiringPiSPISetup falló.\n");
    return(-1);
    }

	sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
	if(sockfd < 0)
		error("Opening socket");

	server.sin_family = AF_INET;		// symbol constant for Internet domain
	server.sin_port = htons(3000);		// port number
	server.sin_addr.s_addr = htonl(INADDR_ANY);	// para recibir de cualquier interfaz de red
    addr.sin_family = AF_INET;		// symbol constant for Internet domain
	addr.sin_port = htons(3000);		// port number
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	// para recibir de cualquier interfaz de red
    length = sizeof(server);
	// binds the socket to the address of the host and the port number
	if(bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0)
		error("Error binding socket.");

	// change socket permissions to allow broadcast
	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
   		error("Error setting socket options\n");


    //instrucciones para ibtener la direccion IP automaticamente
    struct ifreq ifr;
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    //para obtener la IP del wlan0
    strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);
    ioctl(sockfd, SIOCGIFADDR, &ifr);
    IP_adress = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);//guarda el valor de la IP en el puntero IP_adress

    printf("La direccion IP es: %s\n", IP_adress);//imprime el valor de la IP detectado para verificar que sea correcto
    fflush(stdout);
    strcpy(copia_IP,IP_adress);//se copia el valor de la IP en otra variable para poder trabajar con el
    tok = strtok(copia_IP, "."); //se separa por puntos para poder construir la direccion del broadcast
    save_ip = atoi(tok);
    tok = strtok(NULL, ".");
    save_ip1 = atoi(tok);
    tok = strtok(NULL, ".");
    save_ip2 = atoi(tok);
    tok = strtok(NULL, ".");
    save_ip3 = atoi(tok);
    sprintf(broad_ip,"%d.%d.%d.255",save_ip, save_ip1, save_ip2);//se construye la direccion del broadcast
    printf("Mi broadcast es: %s\n", broad_ip);//se imprime el valor construido para ver que sea correcto
    fflush(stdout);

    pthread_create(&thread2, NULL, (void*)&alarma, NULL); //funcion para crear el hilo
    pthread_create(&thread3, NULL, (void*)&ADC_read, NULL); //funcion para crear el hilo
    pthread_create(&thread4, NULL, (void*)&recibir, NULL); //funcion para crear el hilo


    addr.sin_addr.s_addr = inet_addr("192.168.0.110");

    //estructura para adquirir la hora
    struct timeval current_time;
    float adc = 0;
    while(1){
    gettimeofday(&current_time, NULL);
    sprintf(reporte,"%d,%d,%d,%d,%d,%d,%d\n", UTR_identifier,current_time.tv_sec,b1,b2,sw1,sw2,ADCvalue);
    n = sendto(sockfd,reporte, MSG_SIZE, 0, (struct sockaddr *)&addr,length); //se envia el mensaje
        if(n < 0)
            error("sendto");
    printf("%s\n",reporte);
    b1 = 0;
    b2 = 0;
    sw1 = 0;
    sw2 = 0;
    sleep(2);
    }
	return(0);
}
//**********************funciones*******************************************
void handlerb1 (void){
    if (digitalRead (26) == 1){
        debounce = 1;
    }else{
        if (debounce == 1){
            debounce = 0;
            b1=1;
        }
    }
}
void handlerb2 (void){
    if (digitalRead (19) == 1){
        debounce1 = 1;
    }else{
        if (debounce1 == 1){
            debounce1 = 0;
            b2 = 1;
        }
    }
}
void handlers1 (void){
    usleep(100000);
    if (digitalRead (20) == 1){
        sw1 = 1;
    }else{
        sw1 = 2;
    }
}
void handlers2 (void){
    usleep(100000);
    if (digitalRead (21) == 1){
    sw2 = 1;
    }else{
    sw2 = 2;
    }
}
uint16_t get_ADC(int ADC_chan){
uint8_t spiData[2]; // La comunicación usa dos bytes
uint16_t resultado;
// Asegurarse que el canal sea válido. Si lo que viene no es válido, usar canal 0.
if((ADC_chan < 0) || (ADC_chan > 1))
ADC_chan = 0;
// Construimos el byte de configuración: 0, start bit, modo, canal, MSBF: 01MC1000
spiData[0] = 0b01101000 | (ADC_chan << 4);  // M = 1 ==> "single ended"
// C: canal:0 ó 1
spiData[1] = 0; // "Don't care", este valor no importa.
// La siguiente función realiza la transacción de escritura/lectura sobre el bus SPI
// seleccionado. Los datos que estaban en el buffer spiData se sobreescriben por
// los datos que vienen por SPI.
wiringPiSPIDataRW(SPI_CHANNEL, spiData, 2); // 2 bytes
// spiData[0] y spiData[1] tienen el resultado (2 bits y 8 bits, respectivamente)
resultado = (spiData[0] << 8) | spiData[1];
return(resultado);
}
void alarma (void *ptr){
    float adc = 0;
    while (1){
    adc =( (ADCvalue*3.3)/1023);
        if ((ADCvalue >= 155) && (ADCvalue<=775)){
            PWM_off(18);
        }else{

            PWM(18,0.11944575,0.11944575);

        }
    }
}
void PWM (int pin, float del_alto, float del_bajo){ //funcion para hacer un PWM
	switch(var){
		case 0:
			digitalWrite(pin,HIGH);
			var++;
			delay(del_alto);
			break;
		case 1:
			digitalWrite(pin,LOW);
			var = 0;
			delay(del_bajo);
			break;
	}
}
void PWM_off (int pin){//funcion para apagar el pin del PWM
	digitalWrite(pin, LOW);
}
void ADC_read (void *ptr){
    while (1){
        ADCvalue = get_ADC(ADC_CHANNEL);
        /*
        printf("Valor de la conversión: %d\n", ADCvalue);
        fflush(stdout);
        */
        usleep(10000);


    }
}
void recibir (void *ptr){
    while (1){
    memset(buffer, 0,MSG_SIZE);	// se limpia el buffer de entrada
		// receive from a client
    n = recvfrom(sockfd, buffer, MSG_SIZE, 0, (struct sockaddr *)&addr, &length);
    if(n < 0)
        error("recvfrom");
    if (buffer != NULL){
        printf("Se recibio lo siguiente: %s\n", buffer);//se imprime lo que se recibio para ver si es correcto
        fflush(stdout);


    }
    }
}
