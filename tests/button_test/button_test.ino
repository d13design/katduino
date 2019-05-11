#include <SevenSegmentTM1637.h>

// Define display pins and initialize object
const int PIN_CLK = 4;
const int PIN_DIO = 5;
SevenSegmentTM1637 display(PIN_CLK, PIN_DIO);

// Define footswitch pins
const int PIN_SW1 = 10;
const int PIN_SW2 = 11;
const int PIN_SW3 = 12;
const int PIN_SW4 = 13;


void setup() {
  
  // Initialize footswitch pins as inputs
  pinMode(PIN_SW1, INPUT);
  pinMode(PIN_SW2, INPUT);
  pinMode(PIN_SW3, INPUT);
  pinMode(PIN_SW4, INPUT);

  // Initialize display
  Serial.begin(9600);
  display.begin();
  display.setBacklight(100);
  display.setColonOn(false);
  display.print("    STARTING...    ");
  delay(2000);
  display.clear();
  
  display.print("----");

}

void loop() {
  String disp = "";
  
  if(digitalRead(PIN_SW1)==HIGH){
    disp = disp + "1";
  }else{
    disp = disp + "0";
  }

  if(digitalRead(PIN_SW2)==HIGH){
    disp = disp + "2";
  }else{
    disp = disp + "0";
  }

  if(digitalRead(PIN_SW3)==HIGH){
    disp = disp + "3";
  }else{
    disp = disp + "0";
  }

  if(digitalRead(PIN_SW4)==HIGH){
    disp = disp + "4";
  }else{
    disp = disp + "0";
  }

  display.print(disp);
}
