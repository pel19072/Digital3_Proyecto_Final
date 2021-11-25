/*
 ============================================================================
 Name        : UTR.c
 Author      : Carlos Gil y Ricardo Pellecer
 ID Number   : 19443      y 19072
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : UTR para el sistema SCADA
 Assignment  : Proyecto final
 Course      : Electronica Digital 3
 Date        : Guatemala, 24 de septiembbre del 2021
 ============================================================================
 */
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
#include <semaphore.h>
#include <wiringSerial.h>
#include <errno.h>
#define SPI_CHANNEL      0 // Canal SPI de la Raspberry Pi, 0 ó 1
#define SPI_SPEED  1500000 // Velocidad de la comunicación SPI (reloj, en HZ)
                            // Máxima de 3.6 MHz con VDD = 5V, 1.2 MHz con VDD = 2.7V
#define ADC_CHANNEL       0 // Canal A/D del MCP3002 a usar, 0 ó 1
#define  MSG_SIZE 80
//**************************Variables globales*****************************
int debounce3 = 0; //variables para los antirebotes
int debounce1 = 0;
int debounce = 0;
int b1 =0; //variables para los botones y swiches de la UTR
int b2 = 0;
int sw1 = 0;
int sw2 = 0;
int sw3 = 0;
int evento = 0; //variable para almacenar el evento que sucedio
float adc = 0; //variable para guardar la conversion ADC a voltaje
char* toks; //variable para el STRTOK
int led[3]; //arreglo de enteros para el estado de los leds
int c_mensajes = 0; //variable que almacena la cantidad de mensajes que
                    //sucedieron entre cada envio
int UTR_identifier = 2; //identificador de la UTR, puede ser 1 o 2
uint16_t ADCvalue; //variable para guardar el valor del ADC de 0 a 1023
unsigned int length; //variable para guardar el tamaño de la estructura addr
int sockfd, n= 0; //variable para los sockets
char buffer[80]; //buffer de recepcion del los datos del historiador
char* IP_adress; //variable para guardar la IP de la UTR
int boolval = 1;			// for a socket option
char reporte[MSG_SIZE][200]; //arreglo para guardar los datos enviados al
                            //historiador
struct sockaddr_in  server, addr;// estructuras para guardar datos de envio y
                                //recepcion
struct timeval current_time; //estructura para obtener las marcas de tiempo
//*******************prototipos de funciones*********************************
void handlerb1 (void); //prototipos de las funciones para los botones y swiches
void handlerb2 (void);
void handlers1 (void);
void handlers2 (void);
uint16_t get_ADC(int channel); //prototipo de la funcion para obtener el ADC
void PWM_off (int pin); //funcion para apagar el pin PWM
void error(const char *msg); //funcion ERROR
void ADC_read (void *ptr); //hilo para leer el valor del ADC
void recibir (void *ptr); //hilo para recibir datos
void iot(void); //prototipo para la interrupcion con el arduino nano
void comp_adc(void *ptr); //hilo para verificar el valor del ADC
//************************************Hilos***********************************

//**********************************Main*************************************
int main(int argc, char *argv[]) {
  pthread_t thread3, thread4, thread5;//identificador de los hilos
	wiringPiSetupGpio(); //funcion de configuracion para usar el pinout normal
	pinMode(26,INPUT); //pin para el boton como entrada
  pinMode(20,INPUT); //pin para el boton como entrada
  pinMode(21,INPUT); //pin para el boton como entrada
  pinMode(19,INPUT); //pin para el boton como entrada
  pinMode(18,OUTPUT); //pin para el boton como salida
  pinMode(5,OUTPUT); //pin para el boton como salida
  pinMode(6,OUTPUT); //pin para el boton como salida
  pinMode(12,INPUT); //pin para el boton como salida
	pullUpDnControl(26,PUD_DOWN); //pin  configurado en pulldown
  pullUpDnControl(20,PUD_DOWN); //pin configurado en pulldown
  pullUpDnControl(21,PUD_DOWN); //pin configurado en pulldown
  pullUpDnControl(19,PUD_DOWN); //pin configurado en pulldown
  //interrupciones de los botones, swiches y arduino IOT
  wiringPiISR (26, INT_EDGE_BOTH,  (void*) &handlerb1);
  wiringPiISR (19, INT_EDGE_BOTH,  (void*) &handlerb2);
  wiringPiISR (20, INT_EDGE_BOTH,  (void*) &handlers1);
  wiringPiISR (21, INT_EDGE_BOTH,  (void*) &handlers2);
  wiringPiISR (12, INT_EDGE_BOTH,  (void*) &iot);

  sw1 = digitalRead(20); //para que el estado de los swiches comience con el
  sw2 = digitalRead(21); //de la posicion en la que se encuentran
  if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) < 0) { //configuracion del spi
  printf("wiringPiSPISetup falló.\n");
  return(-1);
  }
  if(argc != 2){ //se lee el argumento del main
		printf("usage: %s port\n", argv[0]);
		exit(0);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); // se crea el socket.
	if(sockfd < 0)
		error("Opening socket");

  //se llenan las estructuras de recepcion y envio
  addr.sin_family = AF_INET;		// symbol constant for Internet domain
	addr.sin_port = htons(atoi(argv[1]));		// se coloca el puerto ingresado al
                                          // ejecutar el programa

  server.sin_family = AF_INET;		// symbol constant for Internet domain
	server.sin_port = htons(atoi(argv[1]));		// se coloca el puerto ingresado al
                                            // ejecutar el programa
	server.sin_addr.s_addr = htonl(INADDR_ANY);	// para recibir de cualquier interfaz de red


  length = sizeof(addr);
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
  //guarda el valor de la IP en el puntero IP_adress
  IP_adress = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

  //imprime el valor de la IP detectado para verificar que sea correcto
  printf("La direccion IP es: %s\n", IP_adress);
  fflush(stdout);

  //configuracion UART
  int fd;
  if ((fd = serialOpen("/dev/ttyS0",9600))<0){
      printf("no se pudo abrir el puerto serial %s\n",strerror(errno));
  }

  //funciones para crear los hilos
  pthread_create(&thread3, NULL, (void*)&ADC_read, NULL);
  pthread_create(&thread4, NULL, (void*)&recibir, NULL);
  pthread_create(&thread5, NULL, (void*)&comp_adc, NULL);


    while(1){
    if (evento  == 0){//condicional para guardar datos sin un evento previo
    gettimeofday(&current_time, NULL); //se obtiene el time stamp
    adc =((ADCvalue*3.3)/1023); //se convierte el ADC a voltaje
    //se construye el string que contiene el estado de todos los sensores para
    //enviarlo al historiador
    sprintf(reporte[c_mensajes],"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.2f\n",UTR_identifier,evento,current_time.tv_sec,current_time.tv_usec,b1,b2,sw1,sw2,sw3,led[1],led[0],adc);
    c_mensajes++; //se incrementa la cantidad de mensajes para que si suecede un
                  //evento, el string resultante se guarde en el siguiente espacio
    }
    //ciclo para enviar todos los evento que sucedieron en el rango de 2 segundos
    for (int i = 0; i<c_mensajes ;i++){
    addr.sin_addr.s_addr = inet_addr("10.0.0.23"); //se coloca el addr del hist.
    n = sendto(sockfd,reporte[i], sizeof(reporte[i]), 0, (struct sockaddr *)&addr,length); //se envia el mensaje
    if(n < 0)
        error("error 1 prueba");
    //printf("llegue a enviar\n");
    printf("Se envio: %s",reporte[i]);//se registra lo que se envio al hist.
    serialPuts(fd, reporte[i]); //se envia por serial al ARDUINO para visualizarlo
                                //en blyng
    memset(reporte[i], 0,100); //se vacia el espacio del string enviado
    }

    c_mensajes =0; //se limpia al terminar de enviar todos los strings
    evento = 0; //se actualiza el estado
    sleep(2); //delay para repetir el proceso anterior cada 2 segundos
    }

    serialClose(fd); //se cierra el puerto serial
	  return(0);
}
//**********************funciones*******************************************
void handlerb1 (void){
  //implementacion del antirebote del boton
    if (digitalRead (26) == 1){
        debounce = 1;
    }else{
        if (debounce == 1){
            debounce = 0;
            b1=1;
            //conjunto de instrucciones para registrar el evento
            evento = 2; //se actualiza el evento
            gettimeofday(&current_time, NULL); //se obtiene la marca de timepo
            adc =( (ADCvalue*3.3)/1023); //se convierte el ADc a voltaje
            //se contruye el string para enviar al historiador
            sprintf(reporte[c_mensajes],"%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%.2f\n",UTR_identifier,evento,current_time.tv_sec,current_time.tv_usec,b1,b2,sw1,sw2,sw3,led[1],led[0],adc);
            c_mensajes++; //se incrementa para que el siguiente evento se guarde
                          //en el siguiente espacio
            b1 = 0; //se actualiza el estado del boton, una vez se registra
        }
    }
}
void handlerb2 (void){ //mismo procedimiento que con la fucnion ¨handlerb1¨
    if (digitalRead (19) == 1){
        debounce1 = 1;
    }else{
        if (debounce1 == 1){
            debounce1 = 0;
            b2 = 1;
            evento = 2;
            gettimeofday(&current_time, NULL);
            adc =( (ADCvalue*3.3)/1023);
            sprintf(reporte[c_mensajes],"%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%.2f\n",UTR_identifier,evento,current_time.tv_sec,current_time.tv_usec,b1,b2,sw1,sw2,sw3,led[1],led[0],adc);
            c_mensajes++;
            b2 = 0;
        }
    }
}
void handlers1 (void){
    usleep(100000); //delay para evitar que se lea el rebote del swich
    if (digitalRead (20) == 1){ //condicional para leer y guardar el estado
        sw1 = 1;                //del swich
    }else{
        sw1 = 0;
    }
    evento = 1; //se registra el evento
    gettimeofday(&current_time, NULL); //se registra el momento en el que paso
    adc =( (ADCvalue*3.3)/1023); //se convierte el ADC a voltaje
    //se contruye el string para enviar al historiador
    sprintf(reporte[c_mensajes],"%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%.2f\n",UTR_identifier,evento,current_time.tv_sec,current_time.tv_usec,b1,b2,sw1,sw2,sw3,led[1],led[0],adc);
    c_mensajes++;//se incrementa para que el siguiente evento se guarde
                //en el siguiente espacio
}
void handlers2 (void){ //mismo procedimiento que con la funcion ¨handlers´
    usleep(100000);
    if (digitalRead (21) == 1){
    sw2 = 1;
    }else{
        sw2 = 0;
    }
    evento = 1;
    gettimeofday(&current_time, NULL);
    adc =( (ADCvalue*3.3)/1023);
            sprintf(reporte[c_mensajes],"%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%.2f\n",UTR_identifier,evento,current_time.tv_sec,current_time.tv_usec,b1,b2,sw1,sw2,sw3,led[1],led[0],adc);
    c_mensajes++;
}
void iot(void){ //mismo procedimiento que con la fucnion ¨handlerb1¨
                //en este caso se lee el boton de la interfaz con iot
    if (digitalRead (12) == 1){
        debounce3 = 1;
    }else{
        if (debounce3 == 1){
            debounce3 = 0;
            evento = 3;
            gettimeofday(&current_time, NULL);
            adc =((ADCvalue*3.3)/1023);
            sprintf(reporte[c_mensajes],"%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%.2f\n",UTR_identifier,evento,current_time.tv_sec,current_time.tv_usec,b1,b2,sw1,sw2,sw3,led[1],led[0],adc);
            c_mensajes++;
            fflush(stdout);
            if (sw3 == 0){
                sw3 = 1;
            }else{
                sw3 = 0;
            }
        }
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
void PWM_off (int pin){//funcion para apagar el pin del PWM
	digitalWrite(pin, LOW);
}
void ADC_read (void *ptr){
    //funcion que unicaemenete elee el ADC cada 10000usec
    //se implemento asi para evitar que otras cosasa afectaran la lectura
    while (1){
        ADCvalue = get_ADC(ADC_CHANNEL);
        usleep(10000);
    }
}
void recibir (void *ptr){
    int temp_led[3]; //arreglos de enteros temporales
    int temp1_led[3];
    while (1){
    memset(buffer, 0,MSG_SIZE);	// se limpia el buffer de entrada
    // se reciben los datos del hitoriador y se guardan en buffer
    n = recvfrom(sockfd, buffer, MSG_SIZE, 0, (struct sockaddr *)&addr, &length);
    if(n < 0)
        error("recvfrom");
    if (buffer != NULL ){ //verifica que haya algo en buffer
        //se imprime lo que se recibio para ver si es correcto
        printf("Se recibio lo siguiente: %s\n", buffer);
        fflush(stdout);
        //lo que recibe es ¨LED1,LED2,UTR¨
        toks = strtok(buffer, "."); //se separa el string recibido
        temp_led[0] = atoi(toks);
        toks = strtok(NULL, ".");
        temp_led[1] = atoi(toks);
        toks = strtok(NULL, ".");
        temp_led[2] = atoi(toks);
        if (temp_led[2] == 2){ //se verifica que el mensaje sea para la UTR
            led[0] = temp_led[0]; //se actualizan las variables de los LEDS
            led[1] = temp_led[1];
            digitalWrite(5, led[1]); //se encienden los LEDS respectivos
            digitalWrite(6, led[0]);
            //se verifica   ue lo que se recibio sea distinto a lo que recibio
            //antes
            if (temp1_led[0] != led[0] || temp1_led[1] != led[1] ){
            temp1_led[0] = temp_led[0]; //se actualizan las variables que
            temp1_led[1] = temp_led[1]; //almacenan el estado anterior
            gettimeofday(&current_time, NULL); //se obtiene la hora
            adc =((ADCvalue*3.3)/1023); //se convierte el valor del ADC
            evento = 4; //se registra el evento
            //se construye el string para enviar al historiador
            sprintf(reporte[c_mensajes],"%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%.2f\n",UTR_identifier,evento,current_time.tv_sec,current_time.tv_usec,b1,b2,sw1,sw2,sw3,led[1],led[0],adc);
            c_mensajes++;//se incrementa el valor para que el sigiente evento
                        //se guarde en el siguiente espacio
            }
        }
    }
}
}
void comp_adc(void *ptr){

    int ADC_warning= 0; //variables internas de antirebote del ADC
    int ADC_good=0;
    while(1){
        //se revisa si el valor esta entre 0.5 y 2.5 V
        if ((ADCvalue >= 155) && (ADCvalue<=775)){ //si esta en el ragno
            digitalWrite(18, HIGH); //se apaga la alarma
            ADC_good =1; //se enciende bandera de antirebote
            //condicinal para registrar unicamente el cambio de estado
            if (ADC_warning == 1){
            gettimeofday(&current_time, NULL); //se obtiene el time stamp
            adc =( (ADCvalue*3.3)/1023); //se convierte el adc a voltaje
            evento = 6; //se actualiza el evento
            //se construye el string para enviarlo al historiador
            sprintf(reporte[c_mensajes],"%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%.2f\n",UTR_identifier,evento,current_time.tv_sec,current_time.tv_usec,b1,b2,sw1,sw2,sw3,led[1],led[0],adc);
            c_mensajes++; //se incrementa para que el proximo evento se guarde
                          //en el siguiente espacio
            ADC_warning = 0; //se limpia la bandera del siguiente estado
            }
        }else{ //el adc se sale fuera de los rangos de operacion
            PWM_off(18); //se enciende la alarma
            //misma dinamica que arriba
            ADC_warning = 1;
            if (ADC_good ==1){

            gettimeofday(&current_time, NULL);
            adc =( (ADCvalue*3.3)/1023);
            evento = 5;
            sprintf(reporte[c_mensajes],"%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%.2f\n",UTR_identifier,evento,current_time.tv_sec,current_time.tv_usec,b1,b2,sw1,sw2,sw3,led[1],led[0],adc);
            c_mensajes++;
            ADC_good = 0;
            }
        }
    }
}
