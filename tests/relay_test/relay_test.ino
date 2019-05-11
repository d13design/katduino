#include <SevenSegmentTM1637.h>

#define CH1 2
#define CH2 3
#define PIN_CLK 4
#define PIN_DIO 5

SevenSegmentTM1637 display(PIN_CLK, PIN_DIO);

void setup() {
  // put your setup code here, to run once:
  pinMode(CH1, OUTPUT);
  pinMode(CH2, OUTPUT);

  Serial.begin(9600);
  display.begin();
  display.setBacklight(100);
  display.setColonOn(false);
  display.print("    STARTING....    ");
  delay(3000);
  display.clear();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(CH1, LOW);
  digitalWrite(CH2, LOW);
  display.print("L L");
  delay(2000);
  digitalWrite(CH1, HIGH);
  digitalWrite(CH2, LOW);
  display.print("H L");
  delay(2000);
  digitalWrite(CH1, LOW);
  digitalWrite(CH2, HIGH);
  display.print("L H");
  delay(2000);
  digitalWrite(CH1, HIGH);
  digitalWrite(CH2, HIGH);
  display.print("H H");
  delay(2000);
}
