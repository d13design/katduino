// Include libraries
#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

// Should the display show channels 1, 2, 3 and 4 rather than A.1, A.2, B.1 and B.2 ?
const bool numericDisplay = true;

// Define footswitch pins
#define S1_PIN 10
#define S2_PIN 11
#define S3_PIN 12
#define S4_PIN 13

// Define relay pins
#define CH1 2
#define CH2 3

// Define relay voltages
#define H HIGH
#define L LOW

// Define EEPROM addresses
#define S_ADDR 0
#define S1_MOM_ADDR 10
#define S2_MOM_ADDR 15
#define S3_MOM_ADDR 20
#define S4_MOM_ADDR 25

// Set up the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
uint8_t boss[8]     = {0x17,0x13,0x11,0x1B,0x1F,0x00,0x00,0x00};
uint8_t boss_b[8]   = {0x16,0x11,0x16,0x11,0x1E,0x00,0x00,0x00};
uint8_t boss_o[8]   = {0x0E,0x11,0x11,0x11,0x0E,0x00,0x00,0x00};
uint8_t boss_s[8]   = {0x1F,0x10,0x1F,0x01,0x1F,0x00,0x00,0x00};
uint8_t katana[8]   = {0x1F,0x09,0x09,0x19,0x17,0x00,0x00,0x00};

// Define variables for later use
int curr_state, prev_state, curr_mom;
bool s1_mom, s2_mom, s3_mom, s4_mom;
bool program_mode, was_in_program_mode, released;
unsigned long time_since_last_press = 0;

// Function set up LCD loading screen
void initDisplay(){
  lcd.createChar(0, boss);
  lcd.createChar(1, boss_b);
  lcd.createChar(2, boss_o);
  lcd.createChar(3, boss_s);
  lcd.createChar(4, katana);
  lcd.setCursor(4, 0);
  lcd.write(0);
  lcd.print(" ");
  lcd.write(1);
  lcd.write(2);
  lcd.write(3);
  lcd.write(3);
  lcd.print(" ");
  lcd.write(4);
  lcd.setCursor(0, 1);
  lcd.print("KATANA SWITCH v1");
}

// Function to update the LCD
void setDisplay(){
  // Reset cursor position
  lcd.home();
  // Print channel info
  String chan;
  if(!numericDisplay){
    if(curr_state < 3){
      chan = "A.";
    }else{
      chan = "B.";
    }
    if(curr_state > 2){
      chan = chan + (curr_state/2);
    }else{
      chan = chan + curr_state;
    }
    // Display current amp channel
    lcd.print("AMP CHANNEL: "+chan);
  }else{
    // Display current amp channel
    lcd.print("AMP CHANNEL:    ");
    lcd.setCursor(15, 0);
    lcd.print(curr_state);
  }
  // Set cursor position
  lcd.setCursor(0, 1);
  // Print momentary info
  String mom;
  if(curr_mom == 0){
    lcd.print("MOMENTARY:   OFF");
  }else{
    mom = curr_mom;
    lcd.print("MOMENTARY:");
    lcd.setCursor(15, 1);
    lcd.print(mom);
  }
}

// Function to set the amp channel
void setChannel(int val){
  switch (val){
    case 1:
      digitalWrite(CH1, L);
      digitalWrite(CH2, L);
      break;
    case 2:
      digitalWrite(CH1, H);
      digitalWrite(CH2, L);
      break;
    case 3:
      digitalWrite(CH1, L);
      digitalWrite(CH2, H);
      break;
    case 4:
      digitalWrite(CH1, H);
      digitalWrite(CH2, H);
      break;
    default:
      return;
  }
  prev_state = curr_state;
  curr_state = val;
  EEPROM.write(S_ADDR, curr_state);
  setDisplay();
}

// Function to count how many buttons are currently being pressed
int countSwitches(bool a, bool b, bool c, bool d){
  return ( (a?1:0)+(b?1:0)+(c?1:0)+(d?1:0) );
}

void setup() {
  // Set pin modes
  pinMode(S1_PIN, INPUT);
  pinMode(S2_PIN, INPUT);
  pinMode(S3_PIN, INPUT);
  pinMode(S4_PIN, INPUT);
  pinMode(CH1, OUTPUT);
  pinMode(CH2, OUTPUT);

  // Initialise the LCD
  lcd.begin();
  lcd.backlight();
  initDisplay();
  delay(3000);
  lcd.clear();

  // Set state variables
  curr_state = EEPROM.read(S_ADDR);
  if (curr_state > 4) {
    curr_state = 1;
    EEPROM.write(S_ADDR, curr_state);
  }
  prev_state = curr_state;

  curr_mom = 0;
  // Set momentary variables
  if(EEPROM.read(S1_MOM_ADDR) == 1){
    s1_mom = true;
    curr_mom = 1;
  }else{
    s1_mom = false;
  }
  if(EEPROM.read(S2_MOM_ADDR) == 1){
    s2_mom = true;
    curr_mom = 2;
  }else{
    s2_mom = false;
  }
  if(EEPROM.read(S3_MOM_ADDR) == 1){
    s3_mom = true;
    curr_mom = 3;
  }else{
    s3_mom = false;
  }
  if(EEPROM.read(S4_MOM_ADDR) == 1){
    s4_mom = true;
    curr_mom = 4;
  }else{
    s4_mom = false;
  }
  EEPROM.write(S1_MOM_ADDR, s1_mom);
  EEPROM.write(S2_MOM_ADDR, s2_mom);
  EEPROM.write(S3_MOM_ADDR, s3_mom);
  EEPROM.write(S4_MOM_ADDR, s4_mom);

  // Set the channel (which also updates the display)
  setChannel(curr_state);
  time_since_last_press = millis();
}

void loop() {

  // Read the button states
  bool s1 = digitalRead(S1_PIN) == H;
  bool s2 = digitalRead(S2_PIN) == H;
  bool s3 = digitalRead(S3_PIN) == H;
  bool s4 = digitalRead(S4_PIN) == H;
    
  if(program_mode){
    // Variable to check if the program mode entry buttons have been released
    // We're in progamming mode folks!
    if(!was_in_program_mode){
      // We've just entered program mode so update the display
      lcd.setCursor(13, 1);
      lcd.print("SET");
      // Reset the current momentary settings
      was_in_program_mode = true;
      released = false;
      s1_mom = false;
      s2_mom = false;
      s3_mom = false;
      s4_mom = false;
      delay(800);
    }
    // Wait for a switch to be pressed - and check it's just the one
    if(countSwitches(s1, s2, s3, s4) == 1 && released){
      // Set the new momentary value and commit it to memory
      if(s1){
        s1_mom = true;
        curr_mom = 1;
      }
      if(s2){
        s2_mom = true;
        curr_mom = 2;
      }
      if(s3){
        s3_mom = true;
        curr_mom = 3;
      }
      if(s4){
        s4_mom = true;
        curr_mom = 4;
      }
      EEPROM.write(S1_MOM_ADDR, s1_mom);
      EEPROM.write(S2_MOM_ADDR, s2_mom);
      EEPROM.write(S3_MOM_ADDR, s3_mom);
      EEPROM.write(S4_MOM_ADDR, s4_mom);
      // Turn off programming mode
      program_mode = false;
      // Update the display
      lcd.setCursor(13, 1);
      lcd.print("   ");
      setDisplay();
      delay(1000);
    }else if(countSwitches(s1, s2, s3, s4) == 2 && released){
      if(s1 && s4){
        s1_mom = false;
        s2_mom = false;
        s3_mom = false;
        s4_mom = false;
        EEPROM.write(S1_MOM_ADDR, s1_mom);
        EEPROM.write(S2_MOM_ADDR, s2_mom);
        EEPROM.write(S3_MOM_ADDR, s3_mom);
        EEPROM.write(S4_MOM_ADDR, s4_mom);
        curr_mom = 0;
      }
      //reset
    }else if(countSwitches(s1, s2, s3, s4) == 0){
      released = true;
    }
  }else{
    was_in_program_mode = false;
    // Is a button being pressed
    if(s1 || s2 || s3 || s4){
      // Have we waited long enough to know it's an intentional press?
      if( (millis()-time_since_last_press) > 50) {
        time_since_last_press = millis();
        // Is only one switch being pressed? If so, change the channel
        if(countSwitches(s1, s2, s3, s4) > 1){
          // Multiple buttons are being pressed - are they the right ones for momentary programming?
          if(s1 && s4) program_mode = true;
        }else{
          // Work out which switch is being pressed
          int pressed;
          if(s1) pressed = 1;
          if(s2) pressed = 2;
          if(s3) pressed = 3;
          if(s4) pressed = 4;
          // Change the channel, only if it needs changing
          if(pressed != curr_state) setChannel(pressed);
        }
      }
    }
    delay(200);
  }
  
  
}
