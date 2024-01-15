/*----------------------------------
THINGS TO DO BEFORE RUNNING ON THE MACHINE

IN THIS CODE:
-Properly integrate the I2C code
-Measure the number of steps it takes for the plunger to go from Reset to fully plunged. Set maxPlungeSteps to this value.
-Set correct usernames to match with face ID module ID numbers

ON THE ESP32:
-Measure the distance measured when the sensor is mounted but no cup is present. Set the correct variable to this value.

*/
#pragma config FOSC = INTIO67, FCMEN = OFF, IESO = OFF                      // CONFIG1H
#pragma config PWRT = OFF, BOREN = OFF, BORV = 30                           // CONFIG2L
#pragma config WDTEN = OFF, WDTPS = 32768                                    // CONFIG2H
#pragma config MCLRE = ON, LPT1OSC = OFF, PBADEN = ON, CCP2MX = PORTC       // CONFIG3H
#pragma config STVREN = ON, LVP = OFF, XINST = OFF                          // CONFIG4L
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF                   // CONFIG5L
#pragma config CPB = OFF, CPD = OFF                                         // CONFIG5H
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF               // CONFIG6L
#pragma config WRTB = OFF, WRTC = OFF, WRTD = OFF                           // CONFIG6H
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF           // CONFIG7L
#pragma config EBTRB = OFF                                                  // CONFIG7H
/*#pragma config FOSC = INTIO67
#pragma config WDTEN = OFF, LVP = OFF, MCLRE = OFF*/

#pragma udata   // declare statically allocated uinitialized variables, we may not need this

#include "p18f45k20.h"
#include "08 Interrupts.h"

#include <delays.h>
#include <string.h>	//Used for debugging LCD in MPLAB SIM
#include <stdlib.h>

//#include <iostream>
//#include <std>

//Define commands for LCD
#define CLEAR_SCREEN  	0b00000001
#define FOUR_BIT  		0b00101100
#define LINES_5X7  		0b00111000
#define CURSOR_BLINK  	0b00001111
#define CURSOR_RIGHT  	0b00000110

//Define commands from ESP32
#define CUP_FULL 15
#define RESET 16
#define ERROR_CUP_PRESENT_ON_STARTUP 17
#define CUP_PRESENT 18

#define DATA_PORT  LATD
#define RS_PIN     PORTDbits.RD6
#define E_PIN      PORTDbits.RD7


#define STIRRER_MOTOR PORTAbits.RA4
#define CLEAN_BTN PORTBbits.RB2
#define LIMIT_SWITCH PORTBbits.RB3

//Define stepper motor pins
#define DIR_PIN PORTAbits.RA6
#define STEP_PIN PORTAbits.RA5

//#pragma romdata topRowChar = 0x180
const rom unsigned char topRowChar[15] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5F, 0x03, 0x04, 0x05, 0x06, 0x07, 0xFF, 0xFF};//The list of custom characters for he top row when th cup is filling
const rom unsigned char bottomRowChar[15] = {0x5F, 0x03, 0x04, 0x05, 0x06, 0x07, 0xFF,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};//The list of custom characters for he top row when th cup is filling
const rom unsigned int brewTimeList[8] = {10,20,30,40,50,60,70,80};
//const rom unsigned char username[8] = {"Eddie\x00", "Luke", "Yuvraj", "Zander", "Darren", "Oz", "Glynn", "Guest 1"};

const rom char username0[] = "Eddie";
const rom char username1[] = "Luke";
const rom char username2[] = "Yuvraj";
const rom char username3[] = "Zander";
const rom char username4[] = "Darren";
const rom char username5[] = "Oz";
const rom char username6[] = "Glynn";
const rom char username7[] = "Guest 1";

const rom char *usernames[8] = {&username0, &username1, &username2, &username3, &username4, &username5, &username6, &username7};

int cupPresent = 0;
int plunging = 0;
int brewTime = 0;
int motorResistance = 0;
int systemReady = 0;//Return to 0 after debugging!
int userID = 0;
int ESPinput = 0;	//Set to 0 before releasing
int i = 0;//Used for a for loop later
int mainMenuVar = 0;//Says if the code is at the main menu.
int maxPlungeSteps = 150;//The number of steps the motor takes to get from sealed to fully plunged
int stepsToSeal = 100;//Steps to take the plunger from Home to sealing the chamber
int makingCoffee = 0;
char brewTimeAsChar = "";
char Buffer[16];
char brewTimeBuffer[2];

/** I N T E R R U P T S ***********************************************/

//----------------------------------------------------------------------------
// High priority interrupt vector

#pragma code InterruptVectorHigh = 0x08
void InterruptVectorHigh (void)
{
  _asm
    goto InterruptServiceHigh //jump to interrupt routine
  _endasm
}

/** D E C L A R A T I O N S *******************************************/
#pragma code    // declare executable instructions

void Delay5milli(void)							//Suitable delay for LCD
{
	Delay1KTCYx(2);           						
}


//DEFINE LCD SUBROUTINES---------------------------------------------------------

void SetAddr(unsigned char DDaddr)
{
        DATA_PORT &= 0xf0;                      // Write upper nibble
        DATA_PORT |= (((DDaddr | 0b10000000)>>4) & 0x0f);
                        
        RS_PIN = 0;                             // Set control bit
      	Delay5milli();
        E_PIN = 1;                              // Clock the cmd and address in
        Delay5milli();
        E_PIN = 0;

		DATA_PORT &= 0xf0;                      // Write lower nibble
        DATA_PORT |= (DDaddr&0x0f);

		Delay5milli();
        E_PIN = 1;                              // Clock the cmd and address in
        Delay5milli();
        E_PIN = 0;
}

void WriteCmd(unsigned char cmd)
	{
        DATA_PORT &= 0xf0;
        DATA_PORT |= (cmd>>4)&0x0f;           
        RS_PIN = 0;                     		// Set control signals for command
        Delay5milli();
        E_PIN = 1;                      		// Clock command in
        Delay5milli();
        E_PIN = 0;

       	
        DATA_PORT &= 0xf0;              		// Lower nibble interface
        DATA_PORT |= cmd&0x0f;
		Delay5milli();
        E_PIN = 1;                      		// Clock command in
        Delay5milli();
        E_PIN = 0;
	}

void WriteChar(char data)
{
        DATA_PORT &= 0xf0;
        DATA_PORT |= ((data>>4)&0x0f);

		RS_PIN = 1;                     		// Set control bits
        Delay5milli();
        E_PIN = 1;                      		// Clock nibble into LCD
        Delay5milli();
        E_PIN = 0;

		    
        DATA_PORT &= 0xf0;              		// Lower nibble interface
        DATA_PORT |= (data&0x0f);
        
        Delay5milli();
        E_PIN = 1;                      		// Clock nibble into LCD
        Delay5milli();
        E_PIN = 0;
}

 void WriteString(const rom char *buffer)    
{		 
        while(*buffer)                  		// Write data to LCD up to null
        {
          Delay5milli();
          WriteChar( *buffer);          		// Write character to LCD
          buffer++;                     		// Increment buffer
        }
        return;
}

void WriteInt(int input){
//ONLY validated with two-digit integers
	WriteChar((input / 10) +48);
	WriteChar((input % 10) +48);
}
       
//DEFINE STEPPER MOTOR SUBROUTINES-----------------------------------------------


void stepperForward(void){
	/*
	function: stepperForward
		
	input:    none
		
	output:   none
		
	remarks:  moves the stepper motor by one step (does not set direction)
	*/
	STEP_PIN = 0;			
	Delay1KTCYx(5);
	STEP_PIN = 1;			
	Delay1KTCYx(5);
}


//DEFINE PROGRAM SUBROUTINES---------------------------------------------------

void drawWaterGraphic(int i){
	/*
	function: drawWaterGraphic
		
	input:    int (0-14)
		
	output:   none
		
	remarks:  Draws a picture of a cup containing an amount of liquid corresponding to the input value (0-14).
	*/
	WriteCmd ( CLEAR_SCREEN );    
	SetAddr (0x80);
	WriteChar(0x00);
	WriteChar(topRowChar[i]);
	WriteChar(topRowChar[i]);
	WriteChar(0x01);

	SetAddr (0xC0);
	WriteChar(0x00);
	WriteChar(bottomRowChar[i]);
	WriteChar(bottomRowChar[i]);
	WriteChar(0x02);
}

void mainMenu(void){
	/*
	function: mainMenu
		
	input:    none
		
	output:   none
		
	remarks:  Updates the brew time and displays the user's name and brew time.
	*/
	mainMenuVar = 1;
	brewTime = brewTimeList[userID];
	WriteCmd ( CLEAR_SCREEN );    
	SetAddr (0x80); 
	WriteString("User: ");
	WriteString(usernames[0]);//Write the user's name
	SetAddr (0xC0);//Go to second line
	WriteString("Brew time: ");
	WriteInt(brewTimeList[0]);
	WriteString(" s");
	
}
void resetSystem(void){
	/*
	function: resetSystem
		
	input:    none
		
	output:   none
		
	remarks:  Homes the plunger and resets the system
	*/
	mainMenuVar = 0;
	WriteCmd ( CLEAR_SCREEN );    
	SetAddr (0x80); 
	WriteString("Resetting");
	DIR_PIN = 0;//Set the direction to backward
	while(LIMIT_SWITCH == 0){	//Until the Aeropress reaches the top and presses the limit switch...
		stepperForward();	//Take one step
	}
	systemReady = 1;
	mainMenu();
}

void makeCoffee(void){
	/*
	function: makeCoffee
		
	input:    none
		
	output:   none
		
	remarks:  Seals the chamber, brews and stirs the coffee, and plunges. If called by the cleaning function, brewing and stirring does not occur.
	*/
	mainMenuVar = 0;
	if(systemReady == 1){
		plunging = 1;
		systemReady = 0;
		DIR_PIN = 1;	//Set the stepper motor direction to forward
		WriteCmd ( CLEAR_SCREEN );    
		SetAddr (0x80); 
		WriteString("Please wait");
		for(i=0; i<stepsToSeal; i++){//Swap this out for however long it takes the plunger to seal the top of the chamber.
			stepperForward();
		}
		if(makingCoffee == 1){	//Don't brew if it's cleaning
			STIRRER_MOTOR = 1;

			WriteCmd ( CLEAR_SCREEN );    
			SetAddr (0x80); 
			WriteString("Brewing...");
			for(i=0; i<(brewTimeList[userID]); i++){
				Delay1KTCYx(250);
			}

			STIRRER_MOTOR = 0;
			drawWaterGraphic(0);//Draw an empty cup
		} else {
			WriteCmd ( CLEAR_SCREEN );    
			SetAddr (0x80); 
			WriteString("Cleaning...");
		}
		for(i=0; i<maxPlungeSteps; i++){
			stepperForward();	//Take one step
			if(plunging == 0){	//This For loop is a fail-safe, in case the interrupt to stop the plunging never happens. It's also used for the cleaning function, where it plunges the whole way without filling a cup.
				i=maxPlungeSteps;	//Replace with the number from the For loop.
			}
		}	
		if(makingCoffee == 0){	//If the user is cleaning, don't wait for the cup to be removed
			resetSystem();
		}	
	}
}

void clean(void){
	/*
	function: clean
		
	input:    none
		
	output:   none
		
	remarks:  Prompts users to set up the cleaning process and launches makeCoffee.
	*/
	mainMenuVar = 0;
	while(CLEAN_BTN == 1){
		//Wait for the user to let go of Clean, so it doesn't skip the instruction screen
	}
	WriteCmd ( CLEAR_SCREEN );    
	SetAddr (0x80);
	WriteString("Insert a tablet,");
	SetAddr (0xC0);
	WriteString("press Clean");

	while(CLEAN_BTN == 0){
		//Wait until Clean is pressed again
	}
	makeCoffee();
	
}

void main (void)
{

//SET VARIABLES FOR DEBUG SESSION- DELETE BEFORE FINAL RELEASE!-----------------------------



//SET UP PINS-------------------------------------------------------------------------------

   

   	ANSEL  = 0;	                        	    //turn off all analog inputs
	ANSELH = 0; 
	TRISA  = 0b00000000;                 		//Sets inputs and outputs
   	LATA   = 0b00000000;	                	//Turns off all pins on port A
	TRISB  = 0b00001111;                 		
   	LATB   = 0b11100000;	                	
	TRISC  = 0b00000000;                 		
   	LATC   = 0b00000000;	                	   	
	TRISD  = 0b00000000;                 		//sets PORTd
   	LATD   = 0b00000000;	                	//turns off PORTd outputs, good start position   

//SET UP INTERRUPTS-------------------------------------------------------------------------

	INTCON2bits.RBPU = 0;		// enable PORTB internal pullups
	WPUBbits.WPUB0 = 1;			// enable pull up on RB0

    // ADCON1 is now set up in the InitADC() function.
    TRISBbits.TRISB0 = 1;       // PORTB bit 0 (connected to switch) is input (1)

    // Set up switch interrupt on INT0
    INTCON2bits.INTEDG0 = 0;    // interrupt on falling edge of INT0 (switch pressed)
    INTCONbits.INT0IF = 0;      // ensure flag is cleared
    INTCONbits.INT0IE = 1;      // enable INT0 interrupt

	// Set up global interrupts
    RCONbits.IPEN = 1;          // Enable priority levels on interrupts
    INTCONbits.GIEL = 1;        // Low priority interrupts allowed
    INTCONbits.GIEH = 1;        // Interrupting enabled.

//CONFIGURE DISPLAY-------------------------------------------------------------------------
  
	WriteCmd ( 0x02 );							// sets 4bit operation
	WriteCmd ( CLEAR_SCREEN);		
	WriteCmd ( FOUR_BIT & LINES_5X7 );			// sets 5x7 and multiline operation.
	WriteCmd ( CURSOR_BLINK );					// blinks cursor
	WriteCmd ( CLEAR_SCREEN );

	SetAddr	 ( 0x00 );
	WriteChar ( 0x00 );
	WriteChar ( 0x01 );
	WriteChar ( 0x02 );
	WriteChar ( 0x03 );
	WriteChar ( 0x04 );
	WriteChar ( 0x05 );
	WriteChar ( 0x06 );
	WriteChar ( 0x07 );

	WriteCmd  ( 0x40 );							// WRITING TO LCD MEMORY
	WriteCmd  ( 0b00001011);					//display off
	WriteChar ( 0b00000001 );					// first character
	WriteChar ( 0b00000001 );
	WriteChar ( 0b00000001 );
	WriteChar ( 0b00000001 );
	WriteChar ( 0b00000001 );
	WriteChar ( 0b00000001 );
	WriteChar ( 0b00000001 );
	WriteChar ( 0b00000001 );

	WriteChar ( 0b00010000 );					// second character
	WriteChar ( 0b00010000 );
	WriteChar ( 0b00010000 );
	WriteChar ( 0b00010000 );
	WriteChar ( 0b00011110 );
	WriteChar ( 0b00010010 );
	WriteChar ( 0b00010010 );
	WriteChar ( 0b00010010 );

	WriteChar ( 0b00010010 );					// third character
	WriteChar ( 0b00010010 );
	WriteChar ( 0b00010010 );
	WriteChar ( 0b00011110 );
	WriteChar ( 0b00010000 );
	WriteChar ( 0b00010000 );
	WriteChar ( 0b00010000 );
	WriteChar ( 0b00010000 );

	WriteChar ( 0b00000000 );					// fourth character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );

	WriteChar ( 0b00000000 );					// fifth character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );

	WriteChar ( 0b00000000 );					// fifth character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );


	WriteChar ( 0b00000000 );					// fifth character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );


	WriteChar ( 0b00000000 );					// fifth character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00011111 );

	WriteCmd  ( 0b00001100);	//display back on
	WriteCmd  ( CLEAR_SCREEN);     
	
	
	

//SYSTEM START--------------------------------------------------------------------------
	WriteCmd ( CLEAR_SCREEN );    
	SetAddr (0x80); 
	WriteString("Loading...");
	resetSystem();	//REMOVE AFTER TESTING!!!
	while(systemReady == 0){
		//Wait here until the system has been reset
	}
	while(1){
		while(systemReady == 0){
		//Wait here until the system has been reset
		}
		//Wait for an interrupt
		if(CLEAN_BTN == 1){
			clean();
			/*makingCoffee = 1;//Remove!
			makeCoffee();//Remove!
			makingCoffee = 0;//Remove!*/
		}
		if(cupPresent == 1 && systemReady == 1){
			makingCoffee = 1;
			makeCoffee();
			makingCoffee = 0;
		}
	}
	
		
}



// -------------------- Interrupt Service Routine --------------------------------------
#pragma interrupt InterruptServiceHigh  // "interrupt" pragma also for high priority
void InterruptServiceHigh(void)
{
	if(ESPinput<15){//If it's a water level update
		if(makingCoffee == 1){
			drawWaterGraphic(ESPinput);
		}
	} else if(ESPinput>18){//If it's a user ID
		userID = ESPinput;
		if(mainMenuVar == 1){
			mainMenu();//If it's already on the main menu, reset the main menu to display the new user name.
		}
	} else{//If it's a command from the ESP
		switch(ESPinput){
			case RESET://If the cup is full
				resetSystem();
				break;
			case ERROR_CUP_PRESENT_ON_STARTUP://The cup is present when the ESP starts up
				WriteCmd ( CLEAR_SCREEN );    
				SetAddr (0x80); 
				WriteString("Remove cup");
				break;
			case CUP_PRESENT://The user has inserted a cup
				cupPresent = 1;
				break;
			case CUP_FULL://Stop plunging if the cup is full
				//stepperOff();
				plunging = 0;	//This stops the machine from plunging if it is already plunging.
				WriteCmd ( CLEAR_SCREEN );    
				SetAddr (0x80); 
				WriteString("Remove cup");
				break;
		}
	}
	Delay1KTCYx(10);
	// clear (reset) flag
    INTCONbits.INT0IF = 0;

}
