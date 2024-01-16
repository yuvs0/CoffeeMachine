/******************************************************************************
* | File      	:   PIC_transmit.h
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

#ifndef PIC_transmit
#define PIC_transmit

#ifndef PORT_DEPTH
#define PORT_DEPTH 5
#endif

#include <stdint.h>

//PIC expected commands list
typedef enum PICcommand_t {
  RESET = 14,
  CUP_FULL = 15,
  RESET_DUMMY, //never used
  ERROR_CUP_PRESENT_ON_STARTUP,
  CUP_PRESENT
};

class PIC_TX {
  public:

  //Pin Definition
  uint8_t pins[PORT_DEPTH];
  uint8_t commPin;

  //Port Utility
  void initialisePort (uint8_t pinSet[PORT_DEPTH], uint8_t interruptPin);
  void transmit (uint8_t value);
  void clearPort (void);

  //Custom Transmission
  void sonarTX(uint8_t value);
  void messageTX(PICcommand_t value);
  void faceTX(uint8_t value);
};

#endif