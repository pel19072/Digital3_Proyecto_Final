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

char estado[100];
int state = 0;
WidgetTerminal terminal(V5);
BLYNK_WRITE(V0){
  int V0data = param.asInt();
  digitalWrite(10, V0data);
}
BLYNK_WRITE(V5) // Widget in the app READs Virtal Pin V5 with the certain frequency
{
   
  // This command writes Arduino's uptime in seconds to Virtual Pin V5
  //Blynk.virtualWrite(5, estado);
  //terminal.write(estado);
  //terminal.flush();
}
void setup()
{
  pinMode(10, OUTPUT);
  // Debug console
  Serial.begin(9600);
  Serial1.begin(9600);
  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);


  // Clear the terminal content
  terminal.clear();

  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("-------------"));
  terminal.println(F("Type 'Marco' and get a reply, or type"));
  terminal.println(F("anything else and get it printed back."));
  terminal.flush();
}

void loop()
{
  Blynk.run();
  if (Serial1.available() > 0){
  Serial1.readBytesUntil(10, estado, 41);
  Serial.println(estado);
  terminal.println(estado);
  terminal.flush();
  memset(estado,0,100);
  }
  //delay(2000);
}
