/******************************************************************************
* | File      	:   coffee_sonar.h
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

#ifndef COFFEE_SONAR
#define COFFEE_SONAR

#include <stdint.h>
#include <NewPing.h>
#include "PIC_transmit.h"

extern NewPing sonar;

class coffeeSonar {
  public:

    int pos; //
    int snrDistance; //Distance detected by ultrasonic sensor
    int baseHeight; //The distance between the ultrasonic and the machine base: set this before use.
    int cupHeight;  //The distance between the sensor and the cup that the system is currently callibrated to
    int drinkHeight;  //The desired distance between the sensor and the liquid surface (cup height -5mm)
    float proportionDispensed;  //The amount dispensed on a scale of 0-14
    bool systemReady; //True if there is no cup present
    PIC_TX* pCurrentPIC; //a pointer to a PIC_TX bus instance

  void coffeeSonarInit(PIC_TX *currentPIC);
  void dispenseDrink();
  void resetSystem();

  //utility methods
  uint16_t smoothedSonar(); 
  void transmitMessage(PIC_TX *picBus, PICcommand_t command);
  void transmitSonar(PIC_TX *picBus, uint8_t amount);
  uint8_t coffeeLevel();

};

#endif