/******************************************************************************
* | File      	:   PIC_transmit.cpp
* | Author      :   Yuvraj Sethia, @yuvs0
* | Function    :   Transmit information to a Microchip PIC microcontroller
* | Info        :
*   Default implementation is a 5-bit wide parallel bus with a single comms
*   pin, pulled high when pins are ready to be read, triggering PIC ISR
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

#include "PIC_transmit.h"
#include "Arduino.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>

/*
function: initialisePort

input:    none

output:   none

remarks:  initialises the port by setting all pins in the port to output: mode
*/
void PIC_TX::initialisePort(uint8_t pinSet[PORT_DEPTH], uint8_t interruptPin){
    //Serial.print("interrupt Pin: ");
    //Serial.println(interruptPin);
    commPin = interruptPin;
    //Serial.print("Comm Pin init: ");
    //Serial.println(commPin);
    pinMode(commPin, OUTPUT);
    for (int i = 0; i <= PORT_DEPTH; i++){
        pins[i] = pinSet[i];
        pinMode(pins[i], OUTPUT);
    }     
}

/*
function: transmit

input:    an integer up to max allowed by PORT_DEPTH, e.g. if PORT_DEPTH is 5, max integer is 31

output:   none

remarks:  puts the value - without translation - onto the port defined in property pins (see above)
*/
void PIC_TX::transmit(uint8_t value){
    Serial.println(value);
    for (int i = 0; i < PORT_DEPTH; i++){
        uint8_t expnt = pow(2,i);
        uint8_t highorlow = (value & expnt);
        digitalWrite(pins[i], highorlow);
        ////Serial.println(highorlow);
    }
    digitalWrite(4, HIGH); //pull commPin high to trigger ISR on PIC
    //Serial.print("Comm Pin: ");
    //Serial.println(commPin);
      delay(1000);    //delay to hold signal open
}

/*
function: clearPort

input:    none

output:   none

remarks:  clears the port by resetting all port pins to LOW
*/
void PIC_TX::clearPort(void){
    delay(500);
    digitalWrite(4, LOW); //reset commPin to low, ready for next PIC ISR trigger
    for (int i = 0; i < PORT_DEPTH; i++){
        digitalWrite(pins[i], LOW);
    }     
}

/*
function: sonarTX

input:    an integer 0-14

output:   none

remarks:  sends the number to the PIC through the transmit method
*/
void PIC_TX::sonarTX(uint8_t value){
    transmit(value);
}

/*
function: messageTX

input:    a command of type PICcommand_t

output:   none

remarks:  sends the command to the PIC through the transmit method
*/
void PIC_TX::messageTX(PICcommand_t value){
    transmit(value); //typedef PICcommand_t has built-in transposition
}

/*
function: faceTX

input:    a user ID returned from the TX510 face recognition module. This value is clipped by PORT_DEPTH.

output:   none

remarks:  sends the recognised face ID to the PIC through the transmit method
*/
void PIC_TX::faceTX(uint8_t value){
    if (value <= 254){
        // there is no face ID returned - either an error or all IDs deleted.
    } else {
        transmit(value + 19); //transposing face ID to PIC-side expected range
    }
}