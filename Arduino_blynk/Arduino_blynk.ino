/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

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
//#define BLYNK_TEMPLATE_ID   "8f81bXH7dQDvsG9CVRmniRzZgG9Wumma"


#include <SPI.h>
#include <WiFiNINA.h>
#include <BlynkSimpleWiFiNINA.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "8f81bXH7dQDvsG9CVRmniRzZgG9Wumma";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Pellecer";
char pass[] = "PellecerOApto4";

char estado =0;
int state = 0;
BLYNK_WRITE(V0){
  int V0data = param.asInt();
  digitalWrite(10, V0data);
}
BLYNK_READ(V5) // Widget in the app READs Virtal Pin V5 with the certain frequency
{
   if (Serial.available() > 0){
  estado = Serial.read();
  Serial.print(estado);
  }
  if (estado == 49){
    state = 1;
  }else if (estado == 50){
    state = 2;
  }
  // This command writes Arduino's uptime in seconds to Virtual Pin V5
  Blynk.virtualWrite(5, state);
}
void setup()
{
  pinMode(10, OUTPUT);
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
}

void loop()
{
 
  Blynk.run();
}
