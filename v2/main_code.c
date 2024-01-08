/* DJS 12/02/18 ***********************************************************/
// Originally adapted from LCD.c by DW Smith, 25 January 2012 for 18F45K20 LCD BSc PD&T
// E on D7, RS on D6,  D4-7 on PIC: D0-3.
// V7 DJS 10/2019 V8 DJS 18/02/21 Changed Message Tidied Up
// MCLRE = OFF SO PROG WILL RUN IN RELEASE
// 04/03/21 DJS added CG Capability write Custom Graphics Caracters to LCD initially
// THEN to the screen 04/03/21 REVISED DJS V4 15/10/21 checked 13/10/23 DJS
//*************************************************************************/



#pragma config FOSC = INTIO67
#pragma config WDTEN = OFF, LVP = OFF, MCLRE = OFF
//#pragma udata //declare statically allocated uninitialized variables
//unsigned int topRowChar;
//unsigned int bottomRowChar;



#include "p18f45k20.h"

#include <delays.h>

#define CLEAR_SCREEN  	0b00000001
#define FOUR_BIT  		0b00101100
#define LINES_5X7  		0b00111000
#define CURSOR_BLINK  	0b00001111
#define CURSOR_RIGHT  	0b00000110

#define CUP_FULL 15//ENUMS from I2C input
#define RESET 16
#define ERROR_CUP_PRESENT_ON_STARTUP 17
#define CUP_PRESENT 18


#define DATA_PORT  LATD
#define RS_PIN     PORTDbits.RD6
#define E_PIN      PORTDbits.RD7
#define STIRRER_MOTOR PORTBbits.RB0
#define DOOR_CONTACT PORTBbits.RB1//Change this out for the correct pin
#define CLEAN_BTN PORTBbits.RB2//Change this out for the correct pin
#define LIMIT_SWITCH PORTBbits.RB3//Change this out for the correct pin

//#pragma romdata topRowChar = 0x180
const rom unsigned char topRowChar[14] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5F, 0x03, 0x04, 0x05, 0x06, 0x07, 0xFF};//The list of custom characters for he top row when th cup is filling
const rom unsigned char bottomRowChar[14] = {0x5F, 0x03, 0x04, 0x05, 0x06, 0x07, 0xFF,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};//The list of custom characters for he top row when th cup is filling
const rom unsigned int brewTimeList[8] = {10,20,30,40,50,60,70,80};
const rom unsigned char username[8] = {"Eddie", "Luke", "Yuvraj", "Zander", "Darren", "Oz", "Glynn", "Guest 1"};

int cupPresent = 0;
int plunging = 0;
int brewTime = 0;
int motorResistance = 0;
int systemReady = 0;
int userID = 0;
int ESPinput = 0;
int i = 0;//Used for a for loop later
int mainMenuVar = 0;//Says if the code is at the main menu.



void Delay5milli(void)							//Suitable delay for LCD
{
	Delay1KTCYx(2);           						
}




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
       


void stepperForward(void){
	//Reverse the stepper motor
	//
}

void stepperBackward(void){
	//Reverse the stepper motor
	//
}

void stepperOff(void){
	//Reverse the stepper motor
	//
}

void drawWaterGraphic(int i){

}

void mainMenu(void){
	mainMenuVar = 1;
	brewTime = brewTimeList[userID];
	WriteCmd ( CLEAR_SCREEN );    
	SetAddr (0x80); 
	WriteString("User: ");
	WriteString(username[userID]);//Write the user's name
	SetAddr (0xC0);//Go to second line
	WriteString("Brew time: ");
	WriteString(brewTime);
	WriteString(" s");
	
}
void resetSystem(void){
	mainMenuVar = 0;
	WriteCmd ( CLEAR_SCREEN );    
	SetAddr (0x80); 
	WriteString("Resetting");
	stepperBackward();
	while(LIMIT_SWITCH == 0){
		//Wait until the motor feels resistance
	}
	stepperOff();
	systemReady = 1;
	mainMenu();
}

void makeCoffee(void){
	mainMenuVar = 0;
	if(systemReady == 1){
		plunging = 1;
		systemReady = 0;
		stepperForward();
		Delay10KTCYx(50);//Swap this out for however long it takes the plunger to seal the top of the chamber.
		stepperOff();
		STIRRER_MOTOR = 1;

		WriteCmd ( CLEAR_SCREEN );    
		SetAddr (0x80); 
		WriteString("Brewing...");

		Delay10KTCYx(brewTime);//IMPORTANT: Multiply this by something to turn it into seconds!
		STIRRER_MOTOR = 0;
		drawWaterGraphic(0);//Draw an empty cup
		stepperForward();
		for(i=0; i<1000; i++){//Replace 1000 with the number of ticks it takes to fully plunge the mechanism.
			if(plunging == 0){//This For loop is a fail-safe, in case the interrupt to stop the plunging never happens. It's also used for the cleaning function, where it plunges the whole way without filling a cup.
				i=1000;//Replace with the number from the For loop.
				Delay10KTCYx(10);//Wait 10 counts before checking again
			}
		}
		stepperOff();
			
	}
}

void doorClosed(void){
	mainMenuVar = 0;
	if(cupPresent == 1){
		makeCoffee();
		WriteCmd ( CLEAR_SCREEN );    
		SetAddr (0x80); 
		WriteString("Please remove cup");//This will display until the system resets itself, indictaing the cup has been removed.
	}
}

void clean(void){
	mainMenuVar = 0;
	while(CLEAN_BTN == 0){
		//Wait for the user to let go of Clean
	}
	WriteString("Insert a tablet, close the door and press Clean again");
	while(CLEAN_BTN == 0){
		//Wait until Clean is pressed again
	}
	while(DOOR_CONTACT == 0){
		//Wait for the door to be closed
	}
	makeCoffee();
	
}

void interruptRoutine(void){
	
	if(ESPinput<15){//If it's a water level update
		drawWaterGraphic(ESPinput);
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
				WriteString("Please remove cup");
				break;
			case CUP_PRESENT://The user has inserted a cup
				cupPresent = 1;
				break;
			case CUP_FULL://Stop plunging if the cup is full
				stepperOff();
				plunging = 0;
				WriteCmd ( CLEAR_SCREEN );    
				SetAddr (0x80); 
				WriteString("Please remove the cup");
				break;
		}
	}
}
void main (void)
{

//SET UP

   

   	ANSEL  = 0;	                        	    //turn off all analog inputs
	ANSELH = 0; 
	TRISA  = 0b00000000;                 		
   	LATA   = 0b00000000;	                	
	TRISB  = 0b00000000;                 		
   	LATB   = 0b11100000;	                	
	TRISC  = 0b00000000;                 		
   	LATC   = 0b00000000;	                	   	
	TRISD  = 0b00000000;                 		//sets PORTd
   	LATD   = 0b00000000;	                	//turns off PORTd outputs, good start position   	

	// this code configures the display  
 	
	WriteCmd ( 0x02 );							// sets 4bit operation
	WriteCmd ( CLEAR_SCREEN);		
	WriteCmd ( FOUR_BIT & LINES_5X7 );			// sets 5x7 and multiline operation.
	WriteCmd ( CURSOR_BLINK );					// blinks cursor
	WriteCmd ( CURSOR_RIGHT  );					// moves cursor right	


 // this code configures the display  
  
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
	WriteCmd  ( 0b00001011);	//display off
	WriteChar ( 0b00000000 );					// first character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );

	WriteChar ( 0b00000000 );					// second character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00000000 );

	WriteChar ( 0b00000000 );					// third character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );

	WriteChar ( 0b00000000 );					// fourth character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );

	WriteChar ( 0b00000000 );					// fifth character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );

	WriteChar ( 0b00000000 );					// sixth character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );

	WriteChar ( 0b00000000 );					// seventh character
	WriteChar ( 0b00011111 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );

	WriteChar ( 0b00011111 );					// eigth character
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );
	WriteChar ( 0b00000000 );

	WriteCmd  ( 0b00001100);	//display back on
	WriteCmd  ( CLEAR_SCREEN);     
	
	
	

// Start of user program
WriteCmd ( CLEAR_SCREEN );    
SetAddr (0x80); 
	WriteString("STARTUP");
	while(systemReady = 0){
//Wait until the system has been reset
}



	   
    //SetAddr (0xC0);                             // moves character to begining of second line
    
while (1);				                 		//stop the program - Loop here forever
	
}