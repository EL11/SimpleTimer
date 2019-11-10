//Copyright G. Pasqua 2019 - tanopasqua@hotmail.com

+#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

//--DISPLAY
#define SSD1306_128_32 //Display matrix
#define OLED_ADDR 0x3C //Display address
Adafruit_SSD1306 display(-1); //Reset pin not used


//--I/O
const int buzzer = 8; //+ of piezoelectric buzzer 3-24 VDC. Needs power transistor to erogate up to 24V with external power supply.
const int pot = A1; //Central potentiometer pin to A1 - B1K type suggested, if using a different type please change the potMap threshold at line 195.

//Buffer
int potReadings[200]={}; //Array with untrustable measurements

//Globals
int potVal; //Raw potentiometer value
int potMap; //Mapped potentiometer value
String UOM="SECONDS"; //Unit of measure

//RAM
int pot_past=1; //Initialize past potentiometer measurement
int setTime=0; //Initialize user-set time
unsigned long time; //Initialize memory for program execution time


//Ks
int factor; //Speed at which setTime decreases or increases
int cd_factor; //Resolution of time decrement (1s default)
int buzz_flag=0; //Bool for buzzer activation

//Counters
int count; //Generic counter
int static_counter; //Counter of seconds when potentiometer is static
int parallel_counter; //Parallel counter to use at will




void setup() {
   Serial.begin(9600);
   //Initialize I/O
   pinMode(buzzer,OUTPUT);
   pinMode(pot,INPUT);
   //Initialize display and first screen
   display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR); //Initialize display object
   display.clearDisplay();
   display.setFont(&FreeSerif12pt7b);
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(15, 20);
   display.print("Timer");
   display.display();
   display.setFont();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(90, 14);
   display.print("v1.0"); //VERSION
   display.display();
   //Arbitrary amount of time before start
   delay(5000);
   display.setFont(); //Set font back to default for next use
}

//Display function
void displ(String TXT, String KEY, int COLOR, String KEY2, int COLOR2) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(COLOR);
  display.setCursor(35 , 16);
  display.print(TXT);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(27, 3);
  display.print(KEY);
  display.display();
  //display.setFont(&FreeSans9pt7b); uncomment to change the font for KEY2
  display.setTextSize(1);
  display.setTextColor(COLOR2);
  display.setCursor(97, 5);
  display.print(KEY2);
  display.display();
  display.setFont();
}

//RecordPot methods
void recordPot_staticCheck(){
     if(setTime>0){
        setTime=setTime;
        factor=0;
        if(static_counter==9){ //After 9 seconds starts the countdown
           cd_factor=-1;
      }
      }
     
}

//Record measurements and handle static check
void recordPot(int pot_curr){
     Serial.println("Static counter:" + String(static_counter));
     Serial.println("Factor:" + String(factor));
     Serial.println("Countdown:" + String(cd_factor));
   
     //--CHECKS
     //Check is pot_past has being recorded for the first time
     if(pot_past==1){
        pot_past=pot_curr;
     }
     //Check if value is static, user is not moving the potentiometer for an amount of time (2s default)
     if(static_counter>=2){
      recordPot_staticCheck();
     }
     //Check if countdown has stopped. TODO: distinct between starting zero condition and zero after countdown condition
     if(setTime<=0){
      cd_factor=0;
      buzz_flag=1; //Enable buzzer permission to operate
     }

     //--INCREMENTS
     //Check if potentiometer value is increasing
     if(pot_curr>pot_past){
        if(pot_curr-pot_past<=5){ 
           pot_curr=pot_past;
        }
        buzz_flag=0;
        static_counter=0;
        factor=1; 
        setTime=setTime+factor;   
        pot_past=pot_curr;
        //Accelerate x5 over threshold (15s default)
        if(setTime>15){
          factor=5;
        }
        
      
     }
     //Check if potentiometer value is decreasing
     if(pot_curr<pot_past){
        if(pot_past-pot_curr<=5){
           pot_curr=pot_past;
        }
       factor=0; //Do not decrease setTime, instead stop increment for more accurate control. TODO: implement setTime decreasing
        if(setTime>0){
           setTime=setTime+factor;
        } 
        pot_past=pot_curr;
        
     }

     //Static check
     if(pot_curr==pot_past){
      if(setTime>0){
        setTime=setTime+factor+cd_factor;
      }
      if(static_counter<10){
         static_counter=static_counter+1;
      }
      if(static_counter==10){
         static_counter=0; //Reset static counter on the 10th second
         
      }
     }
     parallel_counter=static_counter;
     
}

//--FILTERs
//Remove first measurements
void garbageFilter(){
  time = millis();
  for (count=0;count<=199;count++){
           potReadings[count]=analogRead(pot);
    }
}

//--DISPLAY VIEW
void handleDisp(){
  displ(String(setTime),UOM,1,String(time/1000),1);
  delay(700); //Main interrupt. If you change this value, check the countdown timing is still correct.
}

void loop() {
    garbageFilter();
    potVal = analogRead(pot);
    UOM="PAUSA";
    if(potVal!=0 && potVal!=1){
       UOM="SECONDI";
       potMap=map(potVal,2,900,1,100); //Map raw potentiometer values to smoother virtual values
       recordPot(potMap); 
       Serial.println(setTime);
       Serial.println(potMap);
       Serial.println("Buzzer:" + String(buzz_flag));
       if(buzz_flag==1){
          tone(buzzer,1000); //Buzzer high pitch
          delay(200); //Timing for buzzer sound. Connected to main interrupt, if you change this value timing may be incorrect.
          tone(buzzer,500); //Buzzer low pitch
     }
       if(buzz_flag==0){
          pinMode(buzzer,LOW);
     }
    }
    handleDisp();
}
