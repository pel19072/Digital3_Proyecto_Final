/*
 ============================================================================
 Name        : Interrupt.c
 Author      : Ricardo Pellecer
 Version     : V1.0
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <wiringPi.h>

// Prototypes
void Handler_1(void);

// Gloabl Variables
int FLAG = 0;

int main(int argc, char *argv[]){
  //Setup necesario
  wiringPiSetup();
  pinMode(25, INPUT); // Botón
  wiringPiISR(25, INT_EDGE_BOTH, (void *)&Handler_1);

  while(1){
  }
  return 0;
}

void Handler_1(void){
  if(digitalRead(25) == 1){
    FLAG = 1;
  }
  else{
    if(FLAG){
      printf("Hola, prueba de interrupt\n");
    }
  }
}
