/******************************************************************************
* | File      	:   coffee_sonar.cpp
* | Author      :   Eddie Carrington, @eddiecarrington1 
* |                 Yuvraj Sethia, @yuvs0
* | Function    :   HC-SR04 NewPing wrapper for Coffee Machine
* | Info        :
*   Default config: Trigger on IO12, Echo on IO13
*   //
*----------------
* |	This version:   V1.0
* | Date        :   2024-01-15
* | Info        :
* -----------------------------------------------------------------------------
* V1.0(2024-01-14):
*   Create library
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documnetation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to  whom the Software is
* furished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/

#include "coffee_sonar.h"
#include "Arduino.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "PIC_transmit.h"

#define PLUNGE_POLL_DELAY_LOOP 120
#define PLUNGE_POLL_DELAY_CHANGE 100
#define SMOOTHING_TOLERANCE 100
#define CONFIG_BASE_HEIGHT 537

/* NewPing Library created by Tim Eckel teckel@leethost.com.
  Copyright 2012 License: GNU GPL v3
  http://www.gnu.org/licenses/gpl-3.0.html
*/
#include <NewPing.h>    //  This calls the NewPing library
#ifndef trigPin
#define trigPin 12  //  Trig pin attached to Arduino 12
#endif

#ifndef echoPin
#define echoPin 13  //  Echo pin attached to Arduino 13
#endif

#define MAX_DISTANCE 15 // Set maximum distance to __cm. The maximum possible is 500cm, but having this variable higher than neccessary slows down the code.
//Base height: 18cm, 991-1043 delay
//Actual height of test cup: About 375
//The sensor is currently giving values up to 144 too low (so thinking the cup is 25mm too high).

#ifdef PIC_transmit
  #define BROADCAST_RESET_SYSTEM transmitMessage(pCurrentPIC, RESET);
  #define BROADCAST_CUP_PRESENT transmitMessage(pCurrentPIC, CUP_PRESENT);
  #define CLEAR_PORT pCurrentPIC->clearPort();
  #define BROADCAST_CUP_FULL transmitMessage(pCurrentPIC, CUP_FULL);
  #define BROADCAST_PROPORTION_DISPENSED transmitSonar(pCurrentPIC, proportionDispensed);
#else
  #define BROADCAST_RESET_SYSTEM Serial.println("Reset broadcast");
  #define BROADCAST_CUP_PRESENT Serial.println("Cup Present");
  #define CLEAR_PORT
  #define BROADCAST_CUP_FULL Serial.println("Cup Full!");
  #define BROADCAST_PROPORTION_DISPENSED Serial.println(proportionDispensed);
#endif

NewPing sonar(trigPin, echoPin, MAX_DISTANCE);

/*
function: coffeeSonarInit

input:    none

output:   none

remarks:  initialisation of class properties
*/
void coffeeSonar::coffeeSonarInit(PIC_TX *currentPIC){
  pos = 20;
  snrDistance = 0;
  baseHeight = CONFIG_BASE_HEIGHT;
  cupHeight = baseHeight;
  drinkHeight = baseHeight;
  proportionDispensed = 0;
  systemReady = 1;
  pCurrentPIC = currentPIC;
}

/*
function: resetSystem

input:    none

output:   none

remarks:  handles resetting of system and class properties
*/
void coffeeSonar::resetSystem(){
  BROADCAST_RESET_SYSTEM
  cupHeight = baseHeight;
  drinkHeight = baseHeight;
  proportionDispensed = 0;
  systemReady = true;
  //Serial.println("Reset complete- ready");
  delay(1000);//Delay to prevent accidental resets
  CLEAR_PORT
  delay(1000);//Delay to prevent accidental resets
}

/*
function: dispenseDrink

input:    none

output:   none

remarks:  handles dispensing of drink
*/
void coffeeSonar::dispenseDrink(){
  //cupHeight = baseHeight;
  //Serial.println("Calibration has started");
  for(int i=1; i<2500; i=i+40){//For 5 seconds
    snrDistance = sonar.ping();
    Serial.println(snrDistance);
    if(snrDistance<cupHeight){
      if(snrDistance != 0){
        cupHeight=snrDistance;
        ////Serial.print("snrDistance = ");
        ////Serial.println(cupHeight);
      }
    }
    delay(75);
  }
  drinkHeight = (cupHeight+25);//Set drink height to 5mm below the rim of the cup. Should be +25, but has been made lower for ease of debugging.
  //Serial.print("Calibration complete. Final cup height: ");
  //Serial.println(cupHeight);
  //Serial.print("Calibration complete. Final drink height: ");
  //Serial.println(drinkHeight);
  //Serial.println("Start plunging");
  BROADCAST_CUP_PRESENT
  snrDistance = sonar.ping();
  //Serial.print("Pre-loop measurement: ");
  //Serial.println(snrDistance);
  delay(500);
  CLEAR_PORT
  while(snrDistance > drinkHeight){
    snrDistance = smoothedSonar();
    //Serial.print("WHILE LOOP");
    delay(PLUNGE_POLL_DELAY_LOOP);
    if(snrDistance){
      if(coffeeLevel()>proportionDispensed){
        //delay(PLUNGE_POLL_DELAY_CHANGE);
        proportionDispensed = coffeeLevel();
        BROADCAST_PROPORTION_DISPENSED
        CLEAR_PORT
      }
    }
  }
  //Serial.println(snrDistance);
  //Serial.print("drink height: ");
  //Serial.println(drinkHeight);
  BROADCAST_CUP_FULL
  //Serial.println("Stop plunging");
  delay(2500);
  CLEAR_PORT
  delay(2500);
}

/*
function: smoothedSonar

input:    PIC_TX* picBus by reference

output:   none

remarks:  sends a message on the PIC parallel bus
*/
uint16_t coffeeSonar::smoothedSonar(){
  uint16_t justMeasured = sonar.ping();
  if ((justMeasured > (snrDistance + SMOOTHING_TOLERANCE)) || (justMeasured < (snrDistance - SMOOTHING_TOLERANCE))){
    //do nothing
  }else{
    return justMeasured;
  }
  return snrDistance;
}

/*
function: transmitMessage

input:    PIC_TX* picBus by reference

output:   none

remarks:  sends a message on the PIC parallel bus
*/
void coffeeSonar::transmitMessage(PIC_TX *picBus, PICcommand_t command){
  picBus->messageTX(command);
}

/*
function: transmitSonar

input:    PIC_TX* picBus by reference

output:   none

remarks:  sends a message on the PIC parallel bus
*/
void coffeeSonar::transmitSonar(PIC_TX *picBus, uint8_t amount){
  picBus->sonarTX(amount);
}

/*
function: coffeeLevel

input:    none

output:   a whole number 0-14 based on current water level

remarks:  calculates a whole number 0-14 based on current water level
*/
uint8_t coffeeSonar::coffeeLevel(){
  int desiredLiquidAmt = (baseHeight)-(drinkHeight);
  int currentLiquidAmount = (baseHeight)-(snrDistance);
  uint16_t bigNumber = 14*currentLiquidAmount;
  int myQuotient = bigNumber/desiredLiquidAmt;
  return myQuotient > 13 ? 13 : myQuotient;
}