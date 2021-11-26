/*
 ============================================================================
 Name        : Arduino_blynk.ino
 Author      : Carlos Gil y Ricardo Pellecer
 ID Number   : 19443      y 19072
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : coneccion del arduino nano con raspi y app de blynk,
               modificacion del ejemplo "Arduino_blynk"
 Assignment  : Proyecto final
 Course      : Electronica Digital 3
 Date        : Guatemala, 24 de septiembbre del 2021
 ============================================================================
 */
/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest
_
  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example shows how to use Arduino MKR 1010
  to connect your project to Blynk.

  Note: This requires WiFiNINA library
    from http://librarymanager/all#WiFiNINA

  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill-in your Template ID (only if using Blynk.Cloud) */
//#define BLYNK_TEMPLATE_ID   "exh7B8ACSPfpESzVMad36SYCaGgkJgBB"


#include <SPI.h>
#include <WiFiNINA.h>
#include <BlynkSimpleWiFiNINA.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "exh7B8ACSPfpESzVMad36SYCaGgkJgBB";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "UVG";
char pass[] = "";

char estado[100]; //arreglo de caracteres para guardar lo que recibe de la raspi
int state = 0; //variable para guardar el estado del LED virtual
WidgetTerminal terminal(V5); //se indica en que pin virtual se coloco la terminal
BLYNK_WRITE(V0){ //handler que se llama cuando el boton de blynk tiene algun cambio
  int V0data = param.asInt(); //se lee el estado del boton virtual
  digitalWrite(10, V0data); //se escribe en el pin del arduino el estado del boton virtual
}

void setup()
{
  pinMode(10, OUTPUT); //se configura el pin D10 como salida
  // Debug console
  Serial.begin(9600); //se configura el serial 0 para imprimir valores en la terminal
  Serial1.begin(9600); //se configura el serial 1 para recibir datos de la raspi
  Blynk.begin(auth, ssid, pass); //se configura la conexion con blynk
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);


  // Clear the terminal content
  terminal.clear();
}

void loop()
{
  Blynk.run(); //se enlaza la comunicacion con blynk
  if (Serial1.available() > 0){
  Serial1.readBytesUntil(10, estado, 41); //se lee hasta el enter lo que se recibe del serial 1
  Serial.println(estado); //se imprime en la terminal para verificar que los datos sean correctos
  terminal.println(estado); //se imprime lo que se recibio en la terminal virtual en blynk
  terminal.flush(); //se vacia la terminal virtual en blynk
  memset(estado,0,100); //se limpia el buffer de recepcion
  }
}
