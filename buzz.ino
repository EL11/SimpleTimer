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
#define SSD1306_128_32 //Matrice display
#define OLED_ADDR 0x3C //Indirizzo display
Adafruit_SSD1306 display(-1); //Reset Pin non utilizzato


//--I/O
const int buzzer = 8;
const int pot = A1;

//Buffer
int potReadings[200]={};

//Globals
int potVal;
int potMap;
String UOM="SECONDI";

//RAM
int pot_past=1;
int setTime=0;
unsigned long time;
unsigned long passedTime;
unsigned long interval;

//Ks
int factor;
int coeff;
int cd_factor;
int buzz_flag=0;

//Counters
int count;
int static_counter;
int parallel_counter;




void setup() {
   Serial.begin(9600);
   pinMode(buzzer,OUTPUT);
   pinMode(pot,INPUT);
   display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
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
   display.print("v1.0"); //VERSIONE
   display.display();
   delay(5000);
   display.setFont();
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
  //display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setTextColor(COLOR2);
  display.setCursor(97, 5);
  display.print(KEY2);
  display.display();
  display.setFont();
}

//RecordoPot methods
void recordPot_staticCheck(){
     if(setTime>0){
        setTime=setTime;
        factor=0;
        if(static_counter==9){
           cd_factor=-1;
      }
      }
     
}

//Confronta misurazione passata e presente, inizia conteggio se valore statico
void recordPot(int pot_curr){
     Serial.println("Static counter:" + String(static_counter));
     Serial.println("Factor:" + String(factor));
     Serial.println("Countdown:" + String(cd_factor));
     //Checks
     
     if(pot_past==1){
        pot_past=pot_curr;
     }
     if(static_counter>=2){
      recordPot_staticCheck();
     }
     
     if(setTime<=0){
      cd_factor=0;
      buzz_flag=1;
     }

     //Increments   
     if(pot_curr>pot_past){
        if(pot_curr-pot_past<=5){ 
           pot_curr=pot_past;
        }
        buzz_flag=0;
        static_counter=0;
        factor=1;
        setTime=setTime+factor;   
        pot_past=pot_curr;
        //Accelerate over threshold
        if(setTime>15){
          factor=5;
        }
        
        
     }
     if(pot_curr<pot_past){
        if(pot_past-pot_curr<=5){
           pot_curr=pot_past;
        }
       //static_counter=0;
       factor=0;
       //cd_factor=0;
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
         static_counter=0;
         
      }
     }
     parallel_counter=static_counter;
     
}

void garbageFilter(){
  time = millis();
  for (count=0;count<=199;count++){
           potReadings[count]=analogRead(pot);
    }
}

void handleDisp(){
  displ(String(setTime),UOM,1,String(time/1000),1);
  delay(700);
}

void loop() {
    garbageFilter();
    potVal = analogRead(pot);
    UOM="PAUSA";
    if(potVal!=0 && potVal!=1){
       UOM="SECONDI";
       potMap=map(potVal,2,900,1,100);
       recordPot(potMap);
       Serial.println(setTime);
       Serial.println(potMap);
       Serial.println("Buzzer:" + String(buzz_flag));
       if(buzz_flag==1){
          tone(buzzer,1000);
          delay(200);
          tone(buzzer,500);
     }
       if(buzz_flag==0){
          pinMode(buzzer,LOW);
     }
    }
    handleDisp();
}
