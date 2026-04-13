#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3f,20,4); 

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
}

void loop() {
  float reading = analogRead(A0);
  float vol = reading*5/1023.0;
  float corr = (vol-2.5)/0.1;
  lcd.setCursor(0,0);
  lcd.print("Corriente : ");
  lcd.setCursor(0,14);
  lcd.print(corr);
  delay(1000);
}
