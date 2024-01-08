/* NewPing Library created by Tim Eckel teckel@leethost.com.
  Copyright 2012 License: GNU GPL v3
  http://www.gnu.org/licenses/gpl-3.0.html
*/
#include <NewPing.h>    //  This calls the NewPing library
#define trigPin 12  //  Trig pin attached to Arduino 12
#define echoPin 13  //  Echo pin attached to Arduino 13
#define MAX_DISTANCE 13 // Set maximum distance to __cm. The maximum possible is 500cm, but having this variable higher than neccessary slows down the code.
//Base height: 18cm, 991-1043 delay
//Actual height of test cup: About 375
//The sensor is currently giving values up to 144 too low (so thinking the cup is 25mm too high).

NewPing sonar(trigPin, echoPin, MAX_DISTANCE);



int pos = 20;
int distance = 0;
int baseHeight = 720;
int cupHeight = baseHeight;
int drinkHeight = baseHeight;
float proportionDispensed = 0;
bool systemReady = 1;

void setup() {
  Serial.begin (115200);//Establish serial communication to 115200 baud
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  if(sonar.ping()<(baseHeight-5.82)){//If the Height detected is over 1mm
    Serial.println("Cup inserted on startup. Please remove the cup.");
    while(sonar.ping()<(baseHeight-5.82)){
      //The code loops here until the cup is removed
    }
    }
    Serial.println("Cup removed");
    delay(2500);//Wait a few seconds so it doesn't start recalibrating as soon as the cup is removed.
    Serial.println("Ready");
  }


void loop() {
  distance = sonar.ping();//Get the distance
  //Serial.println(distance);
  if((distance<(baseHeight-291))&&(distance != 0)){//If the height detected is over 15mm
        if(drinkHeight == baseHeight){//If the system has already been reset
          //Serial.println("Call dispenseDrink");
          //drinkHeight = 10;//Test only, please delete
          systemReady = false;
          dispenseDrink();
        }
  } else if (systemReady == false){//If the system hasn't yet been reset
    if (distance<(baseHeight-5.82)){//If the height detected is less than 1mm (so if the plate is empty)
      //Serial.println("Call resetSystem");
      resetSystem();
      //drinkHeight = 0;//Test only, please delete
    }

    
  }

  delay(40);//Was 450, then 75. 34 is the quickest it can reliably do
}

void resetSystem(){
  cupHeight = baseHeight;
  drinkHeight = baseHeight;
  proportionDispensed = 0;
  systemReady = true;
  Serial.println("Reset complete- ready");
  delay(2000);//Delay to prevent accidental resets
}

void dispenseDrink(){
  //cupHeight = baseHeight;
  Serial.println("Calibration has started");
  for(int i=1; i<2500; i=i+40){//For 5 seconds
    distance = sonar.ping();
    //Serial.println(distance);
    if(distance<cupHeight){
      if(distance != 0){
        cupHeight=distance;
        Serial.print("distance = ");
        Serial.println(cupHeight);
       drinkHeight = (cupHeight+47);//Set drink height to 5mm below the rim of the cup. Should be +47, but has been made lower for ease of debugging.
      }
    }
    delay(75);
  }
  Serial.print("Calibration complete. Final cup height: ");
  Serial.println(cupHeight);
  Serial.println("Start plunging");
  while(distance>drinkHeight or distance == 0){
    distance = sonar.ping();
    delay(75);
    if((floor((14*((float(baseHeight)-float(distance))/(float(baseHeight)-float(drinkHeight))))))>proportionDispensed){
      proportionDispensed = (floor((14*((float(baseHeight)-float(distance))/(float(baseHeight)-float(drinkHeight))))));
      Serial.println(proportionDispensed);
    }
    /*
    //Serial.println("baseHeight = " + baseHeight);
    //Serial.print(" distance = "+distance);
    //Serial.print("drinkHeight = "+drinkHeight);
    Serial.print("baseHeight: ");
    Serial.println(baseHeight);
    Serial.print("distance: ");
    Serial.println(distance);
    Serial.print("drinkHeight: ");
    Serial.println(drinkHeight);
    //Serial.println("((14*((" + baseHeight + "-" distance + ") / (" + baseHeight + "-" + drinkHeight + ")))) = ");
    Serial.print("portionDispensed:");
    proportionDispensed = (floor((14*((float(baseHeight)-float(distance))/(float(baseHeight)-float(drinkHeight))))));
    Serial.println(proportionDispensed);
    Serial.println(" ");
    */
  }
  Serial.println(distance);
  Serial.println("Stop plunging");
  delay(5000);
}
