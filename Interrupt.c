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

int main(int argc, char *argv[]){
  //Setup necesario
  wiringPiSetup();
  pinMode();
  int wiringPiISR(int pin, INT_EDGE_BOTH,  void (*Handler_1)(void));

  while(1){
  }
  return 0;
}
