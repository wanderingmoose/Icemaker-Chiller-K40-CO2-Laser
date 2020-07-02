#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROMex.h>
//#include <EEPROMVar.h>
//float set_temperature;
//float set_temp_swing;
//int set_compressor_delay;

LiquidCrystal_I2C lcd(0x20, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  //LCD stuff
  lcd.init();                      // initialize the lcd
  lcd.clear();
  lcd.setCursor( 0, 0 );   //top left
  lcd.print( "Writting....." );
  lcd.setCursor( 0, 1 );   //bottom left
  // put your setup code here, to run once:
  EEPROM.writeFloat(0, 15);                //Upon menu exit, write these variables to eeprom so we dont loose them at power off or reset.
  lcd.print( "1.." );
  delay(250);
  EEPROM.writeFloat(4, 2);
  lcd.print( "2.." );
  delay(250);
  EEPROM.writeInt(8, 1);
  lcd.print( "3.." );
  delay(250);
  EEPROM.writeFloat(12, 20);
  lcd.print( "4.." );
  delay(250);
  EEPROM.writeFloat(16, 14);
  lcd.print( "5.." );
  delay(250);
  EEPROM.writeInt(20, 1);
  lcd.print( "6.." );
  delay(250);
 lcd.setCursor(0,1);
  EEPROM.writeFloat(24, 1);
  lcd.print( "7.." );
  delay(250);
  EEPROM.writeFloat(28, 1);
  lcd.print( "8.." );
  delay(250);
  EEPROM.writeInt(32, 1);
  lcd.print( "9.." );
  delay(250);
  lcd.print( "Done" );
  delay(2500);
//  set_temperature = EEPROM.readFloat(0);     //Set Temperature is stored in eeprom as a float at address 0 (float 4 bytes long, so next address is 4)
 // set_temp_swing = EEPROM.readFloat(4);      //set Temperature Swing is also stored in eeprom as float at address 4
  //set_compressor_delay = EEPROM.readInt(8);  //set Compressor Delay is stored in eeprom as integer at address 8
  //lcd.clear();
  //lcd.setCursor(0, 0);
  //lcd.print(set_temperature);
  //lcd.setCursor(8, 0);
 // lcd.print(set_temp_swing);
 // lcd.setCursor(0, 1);
 // lcd.print(set_compressor_delay);


}

void loop() {
  // put your main code here, to run repeatedly:

}
