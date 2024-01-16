/******************************************************************************
* | File      	:   coffeeMachine_ESP32firmware.cpp
* | Author      :   Eddie Carrington, @eddiecarrington1 
* |                 Yuvraj Sethia, @yuvs0
* | Function    :   Handles advanced functions of DSC006 Coffee Machine
* | Info        :
*   //
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
// Required Preprocessor Directives
#define PORT_DEPTH 5
#include "PIC_transmit.h"
#include "TX510_uart.h"
#include "coffee_sonar.h"
#define PORT_PIN_0 14
#define PORT_PIN_1 27
#define PORT_PIN_2 26
#define PORT_PIN_3 25
#define PORT_PIN_4 23
extern PICcommand_t PICcommand_t;

uint8_t portPins[PORT_DEPTH] = {
  PORT_PIN_0,
  PORT_PIN_1,
  PORT_PIN_2,
  PORT_PIN_3,
  PORT_PIN_4,
};

// Optional Preprocessor Directives
#define trigPin 12
#define echoPin 13

// Object Instantiation
TX510 tx510;
PIC_TX pic;
extern PIC_TX pic;
coffeeSonar coffeeSonar1;


void setup() {
  Serial.begin (115200);//Establish //Serial communication at 115200 baud
  pic.initialisePort(portPins, 4); //Setup PIC communication port
  coffeeSonar1.coffeeSonarInit(&pic);
  tx510.sendCommand(REGISTRATION);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println(sonar.ping());
  delay(15);

  if(sonar.ping()<(coffeeSonar1.baseHeight-5.82)){//If the Height detected is over 1mm
    //Serial.println("Cup inserted on startup. Please remove the cup.");
    pic.messageTX(ERROR_CUP_PRESENT_ON_STARTUP);
        while(sonar.ping()<(coffeeSonar1.baseHeight-5.82)){
          //The code loops here until the cup is removed
        }
    }
    ////Serial.println("Cup removed");
    delay(2500);//Wait a few seconds so it doesn't start recalibrating as soon as the cup is removed.
    //Serial.println("Ready");
    pic.messageTX(RESET);
    delay(500);//Wait a few seconds so it doesn't start recalibrating as soon as the cup is removed.
    pic.clearPort();
    //Serial.println(coffeeSonar1.baseHeight);
}

void loop() {
  ////Serial.println("loop");
  // put your main code here, to run repeatedly:
  //int id = tx510.sendCommand(RECOGNITION);
  coffeeSonar1.snrDistance = sonar.ping();//Get the distance
  if(coffeeSonar1.snrDistance<(coffeeSonar1.baseHeight-83)){//If the height detected is over 15mm
//
        if(coffeeSonar1.drinkHeight == coffeeSonar1.baseHeight){//If the system has already been reset
          //Serial.println("Call dispenseDrink");
          //drinkHeight = 10;//Test only, please delete
          coffeeSonar1.systemReady = false;
          coffeeSonar1.dispenseDrink();
        }
  } else if (coffeeSonar1.systemReady == false){//If the system hasn't yet been reset
    if (coffeeSonar1.snrDistance<(coffeeSonar1.baseHeight-5.82)){//If the height detected is less than 1mm (so if the plate is empty)
      ////Serial.println("Call resetSystem");      
      coffeeSonar1.resetSystem();
      //drinkHeight = 0;//Test only, please delete
    }

    
  }

  delay(20);//Was 450, then 75. 34 is the quickest it can reliably do. 20 on ESP32
  
}


