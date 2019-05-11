#include <SevenSegmentTM1637.h>
#include <Wire.h>

const int PIN_CLK = 4;
const int PIN_DIO = 5;
SevenSegmentTM1637 display(PIN_CLK, PIN_DIO);

void setup(){
  Wire.begin();
  Serial.begin(9600);
  while (!Serial);
  display.begin();
  display.setBacklight(100);
  display.setColonOn(false);
  display.print("0000");
  delay(3000);
  display.clear();
}

void loop(){
  byte error, address;
  int nDevices;
  String addr;

  display.print("SCAN");
  delay(1000);
  display.clear();

  nDevices = 0;
  for(address = 1; address < 127; address++ ){
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0){
      addr = "";
      if (address<16) 
        addr = addr + "0";
      addr = addr + String(address, HEX);
      display.print(addr);
      delay(3000);
      display.clear();
      nDevices++;
    }else if (error==4){
      addr = "";
      addr = addr + "Unknown error at address 0x";
      if (address<16) 
        addr = addr + "0";
      addr = addr + String(address, HEX);
      addr = addr + " Unknown error at address 0x";
      if (address<16) 
        addr = addr + "0";
      addr = addr + String(address, HEX);
      addr = addr + " Unknown error at address 0x";
      if (address<16) 
        addr = addr + "0";
      addr = addr + String(address, HEX);
      display.print(addr);
      delay(3000);
      display.clear();
    }    
  }
  if (nDevices == 0)
    display.print("NONE");
  else
    display.print("DONE");

  delay(5000);
  display.clear();
}
