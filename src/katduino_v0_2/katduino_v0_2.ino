// Include libraries
#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

/* Here we have a bunch of tailorable settings to get your footswitch working just how you'd like */

// Should we display amp channels as 1, 2, 3 & 4 rather than A1, A2, B1, B2?
const bool numericDisplay = true;

// Define the switches that have to be held down to enter and exit programming mode
// Create a string with 4 characters, each character is a footswitch from 1-4. Use a "1" if you want that button to be pressed and a "0" for unpressed
// Default to enter program mode is switches 1 and 4 together ("1001")
// Default to exit program mode (and turn off momentary) is switches 2 and 4 together ("0101")
const String enterProgramModeCode = "1001";
const String exitProgramModeCode = "0101";
unsigned long timeToWaitForMomentary = 600; // time (in ms) to hold these buttons down for before entering / exiting momentary programming mode

/* End of tailorable settings */

// Define app name and version
const String APP_NAME = "KATDUINO";
const String APP_VERSION = "0.2";

// Define footswitch pins
#define S1 10
#define S2 11
#define S3 12
#define S4 13

// Define the relay pins
#define CH1 2
#define CH2 3

// Define relay voltages
#define H HIGH
#define L LOW

// Define required memory addresses
#define S_ADDR 0
#define S1_MOM_ADDR 10
#define S2_MOM_ADDR 15
#define S3_MOM_ADDR 20
#define S4_MOM_ADDR 25

// Set up the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define some LCD characters for the Boss Katana logo
uint8_t boss[8]     = {0x17,0x13,0x11,0x1B,0x1F,0x00,0x00,0x00};
uint8_t boss_b[8]   = {0x16,0x11,0x16,0x11,0x1E,0x00,0x00,0x00};
uint8_t boss_o[8]   = {0x0E,0x11,0x11,0x11,0x0E,0x00,0x00,0x00};
uint8_t boss_s[8]   = {0x1F,0x10,0x1F,0x01,0x1F,0x00,0x00,0x00};
uint8_t katana[8]   = {0x1F,0x09,0x09,0x19,0x17,0x00,0x00,0x00};

// Define variables for later use
int currState, prevState, currMom; // current amp state and last amp state
unsigned long timeSinceLastPress = 0; // timer to use for debounce testing
unsigned long debounceTime = 100; // time (in ms) to wait before counting a press as an actual press
unsigned long askedForMomentary = 0; // timer to use for momentary programming mode detection
bool inProgrammingMode = false; // are we in programming mode for setting a momentary button
String pressed = "0000"; String lastPressed = "0000"; // strings to use for comparing button states
bool s1Mom, s2Mom, s3Mom, s4Mom; // booleans to track if a button is set to be momentary
int stateBeforeMom = 0; // when the momentary button is down, what should we go back to when it's released
bool momentaryOn = false; // boolean to track if we're currently playing a momentary channel

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
  lcd.setCursor(2, 1);
  String t = APP_NAME + " " + APP_VERSION;
  lcd.print(t);
}

// Function to count how many buttons are currently being pressed
int countSwitches(bool a, bool b, bool c, bool d){
  return ( (a?1:0)+(b?1:0)+(c?1:0)+(d?1:0) );
}

// Function to create comparable strings of button states
String stringPresses(bool a, bool b, bool c, bool d){
  String t = String(a)+String(b)+String(c)+String(d);
  return(t);
}

// Function to update the display
void setDisplay(){
  // Reset the display
  lcd.clear();

  // Display the amp channel
  lcd.print("AMP CHANNEL:");
  String channel = "";
  if(numericDisplay){
    // We're doing a numerical display
    channel = channel + currState;
    lcd.setCursor(15,0);
    lcd.print(channel);
  }else{
    // Were doing an "A1" style display
    if(currState < 3){
      channel = channel + "A" + currState;
    }else{
      channel = channel + "B" + (currState-2);
    }
    lcd.setCursor(14,0);
    lcd.print(channel);
  }
  // Display momentary switch info
  lcd.setCursor(0,1);
  lcd.print("MOMENTARY:");
  if(currMom == 0){
    lcd.setCursor(13,1);
    lcd.print("OFF");
  }else{
    if(currMom == currState){
      lcd.setCursor(14,1);
      lcd.print("ON");
    }else{
      lcd.setCursor(15,1);
      lcd.print(currMom);
    }
  }
  
}

// Function to set the amp channel
void setChannel(int val){
  switch(val){
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
  // Update state variables and commit to memory
  prevState = currState;
  currState = val;
  EEPROM.write(S_ADDR, currState);
  // Update the display
  setDisplay();
}

void setup() {
  // Set up pin modes
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);

  // Set up the LCD
  lcd.begin();
  lcd.backlight();
  initDisplay();
  delay(3000);
  lcd.clear();

  // Load stored settings from memory
  currState = EEPROM.read(S_ADDR);
  if(currState > 4) currState = 1;
  prevState = currState;
  currMom = 0;
  if(EEPROM.read(S1_MOM_ADDR) == 1){
    s1Mom = true;
    currMom = 1;
  }else{
    s1Mom = false;
  }
  if(EEPROM.read(S2_MOM_ADDR) == 1){
    s2Mom = true;
    currMom = 2;
  }else{
    s2Mom = false;
  }
  if(EEPROM.read(S3_MOM_ADDR) == 1){
    s3Mom = true;
    currMom = 3;
  }else{
    s3Mom = false;
  }
  if(EEPROM.read(S4_MOM_ADDR) == 1){
    s4Mom = true;
    currMom = 4;
  }else{
    s4Mom = false;
  }

  // REMOVE THIS WHEN PROGRAMMING MODE IS DONE
  s1Mom = false;
  s2Mom = false;
  s3Mom = true;
  s4Mom = false;
  currMom = 3;
  // END
  
  // Update memory settings just in case they weren't set before
  EEPROM.write(S_ADDR, currState);
  EEPROM.write(S1_MOM_ADDR, s1Mom);
  EEPROM.write(S2_MOM_ADDR, s2Mom);
  EEPROM.write(S3_MOM_ADDR, s3Mom);
  EEPROM.write(S4_MOM_ADDR, s4Mom);
  
  // Set the starting amp channel (which also updates the display)
  stateBeforeMom = currState;
  setChannel(currState);
  
  // Start the debounce timer
  timeSinceLastPress = millis();
}

void loop() {
  // Get the latest state of all buttons
  bool s1Pressed = digitalRead(S1) == H;
  bool s2Pressed = digitalRead(S2) == H;
  bool s3Pressed = digitalRead(S3) == H;
  bool s4Pressed = digitalRead(S4) == H;
  pressed = stringPresses(s1Pressed, s2Pressed, s3Pressed, s4Pressed);

  // Are we in programming mode or not?
  if(!inProgrammingMode){ // Not in programming mode
    
    // If the switch has changed, reset the timer
    if(pressed != lastPressed){
      timeSinceLastPress = millis();
      if(pressed == enterProgramModeCode){
        // The "enter momentary programming mode" switches are being pressed
        askedForMomentary = millis();
      }else{
        askedForMomentary = 0;
      }
    }

    // Check to see if we should enter momentary programming mode or not
    if(askedForMomentary == 0){ // Business as usual
      // We want to have had the buttons in the same state for {debounceTime} milliseconds before counting it as an actual press
      if((millis() - timeSinceLastPress) > debounceTime){
        // Yay! We should count this as an actual button press!
        if(pressed == "0000" && currMom != 0 && pressed != lastPressed && momentaryOn){
          // User has released all buttons and is in momentary mode - time to reset
          momentaryOn = false;
          setChannel(stateBeforeMom);
        }else{
          if(countSwitches(s1Pressed, s2Pressed, s3Pressed, s4Pressed)==1){ // Only one button is being pressed
            // Work out which button has been pressed
            int whichButton;
            if(s1Pressed) whichButton = 1;
            if(s2Pressed) whichButton = 2;
            if(s3Pressed) whichButton = 3;
            if(s4Pressed) whichButton = 4;
            // If button is different to current state, switch channels
            if(whichButton != currState){
              // If a momentary button is set, and is being pressed we should switch on momentary mode
              if(whichButton == currMom){
                stateBeforeMom = currState;
                momentaryOn = true;
              }
              setChannel(whichButton);
            }
          }
        }
      }
    }else{ // We're being timed for entering momentary setting mode
      if((millis() - askedForMomentary) > timeToWaitForMomentary){
        // We've held the "enter programming mode" buttons down for long enough
        inProgrammingMode = true;
        lcd.clear();
        lcd.print("PROGRAMMING MODE");
        lcd.setCursor(0,1);
        lcd.print("SET MOMENTARY: ");
        lcd.blink();
      }
    }
    
  }else{ // We are in programming mode
    
    // If the switch has changed, reset the timer
    if(pressed != lastPressed){
      timeSinceLastPress = millis();
      if(pressed == exitProgramModeCode){
        // The "exit momentary programming mode" switches are being pressed
        askedForMomentary = millis();
      }else{
        askedForMomentary = 0;
      }
    }

    // Check to see if we should exit momentary programming mode or not
    if(askedForMomentary == 0){ // Business as usual we'll let the user choose a momentary button
      // We want to have had the buttons in the same state for {debounceTime} milliseconds before counting it as an actual press
      if((millis() - timeSinceLastPress) > debounceTime){
        // Yay! We should count this as an actual button press!
        if(countSwitches(s1Pressed, s2Pressed, s3Pressed, s4Pressed)==1){ // Only one button is being pressed
          // Work out which button has been pressed
          int whichButton;
          if(s1Pressed) whichButton = 1;
          if(s2Pressed) whichButton = 2;
          if(s3Pressed) whichButton = 3;
          if(s4Pressed) whichButton = 4;
          // If the momentary channel has changed...
          if(currMom != whichButton){
            // Update the current momentary
            currMom = whichButton;
            // Store the current momentary
            s1Mom = false;
            s2Mom = false;
            s3Mom = false;
            s4Mom = false;
            if(currMom == 1) s1Mom = true;
            if(currMom == 2) s2Mom = true;
            if(currMom == 3) s3Mom = true;
            if(currMom == 4) s4Mom = true;
            EEPROM.write(S1_MOM_ADDR, s1Mom);
            EEPROM.write(S2_MOM_ADDR, s2Mom);
            EEPROM.write(S3_MOM_ADDR, s3Mom);
            EEPROM.write(S4_MOM_ADDR, s4Mom);
            // Update the programming mode display
            lcd.setCursor(15,1);
            lcd.print(currMom);
            delay(1500);
            // Exit programming mode
            inProgrammingMode = false;
            // Reset the display
            setDisplay();
          }
        }
      }
    }else{ // We're being time for exiting momentary setting mode
      if((millis() - askedForMomentary) > timeToWaitForMomentary){
        // We've held the "exit programming mode" buttons down for long enough
        currMom = 0;
        s1Mom = false;
        s2Mom = false;
        s3Mom = false;
        s4Mom = false;
        // Reset memory settings
        EEPROM.write(S1_MOM_ADDR, s1Mom);
        EEPROM.write(S2_MOM_ADDR, s2Mom);
        EEPROM.write(S3_MOM_ADDR, s3Mom);
        EEPROM.write(S4_MOM_ADDR, s4Mom);
        lcd.clear();
        lcd.print("PROGRAMMING MODE");
        lcd.setCursor(1,1);
        lcd.print("MOMENTARY OFF!");
        delay(1500);
        inProgrammingMode = false;
        setDisplay();
      }
    }
    
  }

  lastPressed = pressed;
}
