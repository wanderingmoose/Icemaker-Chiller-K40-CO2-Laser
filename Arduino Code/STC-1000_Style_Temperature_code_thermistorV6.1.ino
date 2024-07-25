

/*
   June 30,2020 "Wanderingmoose Tinkering"
   *****IceMaker K40 Chiller Controller ****
           ***** V 6.10 Final *****
             This version changed epprom from write to update to increase eeprom life****
  STC-1000 Style cooling temperature controller (found on a help site, do not remember where, might have been a forum)
  Features:
  Save setpoints in eeprom for temp, swing, compressor timer and other menu items.
  Recommend using a seperate program to format eeprom before loading this program. *****or results may vary*****
  Recommend makeing a script and loading into the Arduino to write the values needed, then loading this script. One is supplied below.
  Simple code to write the eeprom for the first time. ONLY NEED TO LOAD THIS SCRIPT ONCE IN THE ARDUINO to format the eeprom.
              ----------------------------------------------------------------------------------------
  -----copy into new window and load into arduino. only takes seconds to achieve,  Then load the main program into the Arduino.-------
  #include <EEPROMex.h>
  #include "Arduino.h"
  void setup()
    { 
  EEPROM.writeFloat(0, 20);   //address 0
  EEPROM.writeFloat(4, 2);    //address 4
  EEPROM.writeInt(8, 30);     //address 8
  EEPROM.writeFloat(12, 25);  //address 12
  EEPROM.writeFloat(16, 15);  //address 16
  EEPROM.writeInt(20, 1);     //address 20
  EEPROM.writeInt(24, 1);     //address 24
  EEPROM.writeInt(28, 1);     //address 28
  EEPROM.writeInt(32, 1);     //address 32  
    }
  void loop()
  { // Nothing to do during loop }
  ---------------------------------------------------------------------------------------------------------------------------
            ----------------------------------------------------------------------------------------
  ***Outputs***
  Output for Alarm relay and LO disable laser
  Output for Buzzer
  Output for compressor control
  VGA Display support using OctaPentaVeega board. Pin 8 serial output(32x16 character display using VGA monitor).
    -Great for across the room viewing or blind people like me!
  ***Inputs****
  Four switch pad so not using the left button. But still works great for me. (designed for a 5 button switches)
  Pulse flow meter
  2xThermistor 10k. First Temp is for cooling compressor control, the second is for visual info.
              +5volts
              |
              |
              10k Thermister
              |
              |
  Analog Ax---
              |
              |
              10k ohm resistor
              |
              |
              GND
  ***Libraries***
  EEPROMex for eeprom https://playground.arduino.cc/Code/EEPROMex/
  Thermistor used https://github.com/panStamp/thermistor
  SoftwareSerial.h generic one in the interface
  OctaPentaVeega module can be found at https://github.com/rakettitiede/octapentaveega
  Built the B&W module for this project and have the information on the VGA monitor. Pretty slick.


           +5v
              |
             2k2
              |
  Analog A0---   ---- Button1 ---- |
              |                   |
            620ohm                |
              |                   |
                ---- Button2 ---- |
              |                   |
             1k2                  |
              |                   |
                ---- Button3 ---- |
              |                   |
             3k3                  |
              |                   |
              | ---- Button4 ---- |
                                  |
                                  |
                                Ground

  Flow meter is a pulse in on arduino Pin 2.
  Door(laser safety control) switch is on Pin 7.
  Relay(laser safety control) out are on pin 6.
  Buzzer/ audio alarm is on pin 5.
  A2 is for water flow temp coming from LAser tube cooling.
  A1 I used it for just a reference or main resivor temp. (could add in code and make a dew point values aswell...I didn't)
  VGA module is on Pin 8.
  Status LED 13.

  Go through the code and double check the info above. 
  
  Menu: All done with code in the main loop
  Set Temperature--> Set Temp--> Swing Set--> Comp Delay--> High Temp Alarm--> Low Temp Alarm--> Min Flow--> Door--> Buzzer--> Alarms--> EXIT



*/
/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/
#include <EEPROMex.h>          // include expanded eeprom library
#include <thermistor.h>       // For temperature sensors
#include <SoftwareSerial.h> //for VGA Display
/*--------------------------------------------------------------------------------------
  Defines
  --------------------------------------------------------------------------------------*/
//Temperature pins
#define tempPin1                 A2  //A2 is the input for Thermistor 10k Water reading of water coming from laser into cooling system.
#define tempPin2                 A1  // Just a temperature reading for water leaving tank or anyplace else you wish.
#define dOorIn                   7   //Door switch input 
// Pins in use
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input keyboard input
#define COOL_PIN                 4   //Cooling Relay Pin
#define alarm_PIN                6   //Alarm relay and LO disable laser.
#define buzzer_PIN               5   //Buzzer pin
// Pin 2 is for Flow meter
// ADC readings expected for the 5 buttons on the ADC input 10bit
// Four switch input on A0. Not using left button for this sketch
#define RIGHT_10BIT_ADC          50  // right
#define UP_10BIT_ADC            741  // up
#define DOWN_10BIT_ADC          522  // down
#define LEFT_10BIT_ADC          1555 // left not using so made big
#define SELECT_10BIT_ADC        250  // right
//
#define BUTTONHYSTERESIS         50  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  // 
#define BUTTON_RIGHT              1  // 
#define BUTTON_UP                 2  // 
#define BUTTON_DOWN               3  // 
#define BUTTON_LEFT               4  // 
#define BUTTON_SELECT             5  // 
//Menu tree
#define MENU_SETTEMP              0
#define MENU_TEMP_SWING           1
#define MENU_COMPRESSOR_DELAY     2
#define MENU_HIGH_TEMP_ALARM      3
#define MENU_LOW_TEMP_ALARM       4
#define MENU_MIN_FLOW             5
#define MENU_DOOR                 6
#define MENU_BUZZER               7
#define MENU_ALARMS               8
#define MENU_EXIT                 9
// VGA Display stuff
SoftwareSerial vga(9, 8);   // We are talking to OctaPentaVeega through pin 8.
// Pin 9 is unused as the shield does not respond. Open loop system.
// Some static stuff for ANSI codes to the Veega board.
const byte ESC[2] = {0x1B, 0x5B};
const byte CLR[4] = {0x1B, 0x5B, 0x32, 0x4A};
// Some static stuff for drawing a box on the UI - codes are graphic characters.
const byte UPBOX[32] = {137, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 151};
const byte MIDBOX[32] = {138, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 138};
const byte LOWBOX[32] = {136, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 135};
const byte CROSSBOX[32] = {144, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 140};

/*--------------------------------------------------------------------------------------
  Variables
  --------------------------------------------------------------------------------------*/
byte buttonJustPressed  = false;         //this will be true after a ReadButtons() call if triggered
byte buttonJustReleased = false;         //this will be true after a ReadButtons() call if triggered
byte buttonWas          = BUTTON_NONE;   //used by ReadButtons() for detection of button events
float tempReading;                       //analog reading from sensor
float tempReading1;
float ipvdc = 0.0;
float flow = 0.0;
byte temp = 0;
byte temp1 = 0;
int compressor_time_temp = -300;         //default to a time that will allow compressor to turn on the first time run.
int compressor_time;
float temptotal = 0;
float temptotal1 = 0;
int avgnum = 30;
int avgnum1 = 10;
float tempavg = 0;
float tempavg1 = 0;
boolean menu = true;
boolean set_temp = true;
boolean temp_swing = true;
boolean compressor_delay = true;
boolean compressor_flag = false;
boolean high_temp  = true;
boolean low_temp  = true;
boolean min_flow  =   true;
boolean door  = false;
boolean buzzer  = false;
boolean alarms  = false;
int menu_number = 0;
float set_temperature;
float set_temp_swing;
float set_high_alarm;
float set_low_alarm;
float set_min_flow;
int set_door;
int set_buzzer;
int set_alarms;
int set_compressor_delay;
boolean a = LOW, b = HIGH;
float spread;
int hiGh = 0;
int lOw = 0;
int mIn = 0;
int doOr = 0;
int buzZer = 0;
int alaRms = 0;
float tempAlarm = 0;
char* MENU[] = {"Set Temperature?", "Set Temp Swing ?", "Set Comp Delay ?", "High Temp Alarm?", "Low Temp Alarm ?", "Min Flow       ?", "Door           ?", "Buzzer         ?", "Alarms         ?", "EXIT           ?"}; //  Just an array of menu items we can index with menu #  ADD OPTION FOR SAVE NEW VALUES AND EXIT
// Thermistor object
THERMISTOR thermistor1(tempPin1,        // Analog pin
                       10000,           // Nominal resistance at 25 ºC
                       3950,            // thermistor's beta coefficient
                       10000);          // Value of the series resistor
THERMISTOR thermistor2(tempPin2,        // Analog pin
                       10000,           // Nominal resistance at 25 ºC
                       3950,            // thermistor's beta coefficient
                       10000);          // Value of the series resistor
byte statusLed    = 13;
byte sensorInterrupt = 0;  // 0 = digital pin 2 flow meter input
byte sensorPin       = 7; //
// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;
volatile byte pulseCount;
float flowRate;
unsigned long oldTime;
/*--------------------------------------------------------------------------------------
   setup()
  Called by the Arduino framework once, before the main loop begins
  --------------------------------------------------------------------------------------*/
void setup()
{
  analogReference(DEFAULT);
  pinMode( BUTTON_ADC_PIN, INPUT );          //ensure A0 is an input
  pinMode( tempPin1, INPUT);                 //ensure A1 is an input
  digitalWrite( BUTTON_ADC_PIN, LOW );       //ensure pullup is off on A0
  digitalWrite( tempPin1, LOW );             //ensure pullup is off on A1
  pinMode(dOorIn, INPUT_PULLUP);             //requires a 10k-20k resistor to +5vdc to work properly
  pinMode(COOL_PIN, OUTPUT);                 //Compressor control
  pinMode(alarm_PIN, OUTPUT);                //Laser fire control
  pinMode(buzzer_PIN, OUTPUT);
  digitalWrite(alarm_PIN, LOW);
  digitalWrite(COOL_PIN, LOW);
  digitalWrite(buzzer_PIN, LOW);
  delay(5000);
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  pulseCount        = 0;
  flowRate          = 0.0;
  oldTime           = 0;
  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  //***************************************************************************************STORING CERTAIN CONSTANTS IN EEPROM IN CASE OF POWER OUTAGE ETC...  ******************************************************************************************************
  set_temperature = EEPROM.readFloat(0);       //Set Temperature is stored in eeprom as a float at address 0 (float 4 bytes long, so next address is 4)
  set_temp_swing = EEPROM.readFloat(4);        //set Temperature Swing is also stored in eeprom as float at address 4
  set_compressor_delay = EEPROM.readInt(8);    //set Compressor Delay is stored in eeprom as integer at address 8
  set_high_alarm = EEPROM.readFloat(12);       //address 12
  set_low_alarm = EEPROM.readFloat(16);        //address 16
  set_min_flow =  EEPROM.readInt(20);          //address 20
  set_door = EEPROM.readInt(24);               //address 24
  set_buzzer = EEPROM.readInt(28);             //address 28
  set_alarms = EEPROM.readInt(32);             //address 32



  //VGA Display Stuff
  vga.begin(9600);             // start the serial port for OctaPentaVeega
  vga.write (CLR, 4);          // clear the VGA screen
  InitScreen();                // draw the initial UI elements
  SetNoWrap();                 // set nowrap on VGA
}
/*--------------------------------------------------------------------------------------
  loop()
  Arduino main loop
  --------------------------------------------------------------------------------------*/
void loop()
{
  byte button;
  byte buttonwas;
  byte timestamp;
  SetCursor (13, 1);
  vga.print ("                ");
  SetColor (37, 40);
  SetCursor (13, 14);
  vga.print ("MENU");
  //Check to see if we need to enter Menu.  Only button that will do this is the "Select" Button
  button = ReadButtons();  //This calls ReadButtons Function Found Below
  switch ( button )
  {
    //**********************************************************************************TOP LEVEL MENU*******************************************************************************************************************************************************
    case BUTTON_SELECT:
      {
        menu = true;                               //Make sure menu while loop flag is set true
        menu_number = 0;                           //Default first menu item to 0 case
        delay(200);
        while (menu == true)                       //Stay in menu as long as menu flag is true
        {
          // digitalWrite( LCD_BACKLIGHT_PIN, HIGH);
          button = ReadButtons();                       //Determine which part of menu to enter, or exit menu.  Only uses Left Right, and Select Buttons
          switch ( button)
          {
            case BUTTON_LEFT:
              {
                menu_number = menu_number - 1;
                if (menu_number < 0 ) menu_number = 9;
                delay(200);
                break;
              }
            case BUTTON_RIGHT:
              {
                menu_number = menu_number + 1;
                if (menu_number > 9) menu_number = 0;
                delay(200);
                break;
              }
            case BUTTON_SELECT:
              {
                delay(200);
                switch (menu_number)
                {
                  //************************************************************************************************************SET TEMP******************************************************************************************************************************************
                  case MENU_SETTEMP:
                    {
                      set_temp = true;
                      while (set_temp == true)
                      {
                        SetCursor (14, 3);
                        vga.print ("Set Temperature");
                        SetCursor (14, 22);
                        vga.print (set_temperature);
                        button = ReadButtons();
                        switch (button)
                        {
                          case BUTTON_UP:
                            {
                              set_temperature = set_temperature + .5;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_temperature);
                              delay(200);
                              break;
                            }
                          case BUTTON_DOWN:
                            {
                              set_temperature = set_temperature - .5;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_temperature);
                              delay(200);
                              break;
                            }
                          case BUTTON_SELECT:
                            {
                              SetCursor (13, 2);
                              vga.print ("                          ");
                              SetCursor (14, 22);
                              vga.print ("        ");
                              set_temp = false;
                              delay(200);
                              break;
                            }
                            break;
                        }
                      }
                      break;
                    }
                  //**************************************************************************************************************SET TEMP SWING*********************************************************************************************************************************
                  case MENU_TEMP_SWING:
                    {
                      temp_swing = true;
                      while (temp_swing == true)
                      {
                        SetCursor (14, 3);
                        vga.print ("Set Swing");
                        SetCursor (14, 22);
                        vga.print (set_temp_swing);
                        button = ReadButtons();
                        switch (button)
                        {
                          case BUTTON_UP:
                            {
                              set_temp_swing = set_temp_swing + .5;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_temp_swing);
                              delay(200);
                              break;
                            }
                          case BUTTON_DOWN:
                            {
                              set_temp_swing = set_temp_swing - .5;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_temp_swing);
                              delay(200);
                              break;
                            }
                          case BUTTON_SELECT:
                            {
                              SetCursor (13, 2);
                              vga.print ("                          ");
                              SetCursor (14, 22);
                              vga.print ("        ");
                              temp_swing = false;
                              delay(200);
                              break;
                            }
                            break;
                        }
                      }
                      break;
                    }
                  //******************************************************************************************************************SET COMPRESSOR DELAY*************************************************************************************************************************************
                  case MENU_COMPRESSOR_DELAY:
                    {
                      compressor_delay = true;
                      while (compressor_delay == true)
                      {
                        SetCursor (14, 3);
                        vga.print ("Set Comp Delay");
                        SetCursor (14, 22);
                        vga.print (set_compressor_delay);
                        button = ReadButtons();
                        switch (button)
                        {
                          case BUTTON_UP:
                            {
                              set_compressor_delay = set_compressor_delay + 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_compressor_delay);
                              delay(200);
                              break;
                            }
                          case BUTTON_DOWN:
                            {
                              set_compressor_delay = set_compressor_delay - 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_compressor_delay);
                              delay(200);
                              break;
                            }
                          case BUTTON_SELECT:
                            {
                              SetCursor (13, 2);
                              vga.print ("                          ");
                              SetCursor (14, 22);
                              vga.print ("        ");
                              compressor_delay = false;
                              delay(200);
                              break;
                            }
                            break;
                        }
                      }
                      break;
                    }
                  //******************************************************************************************************************SET HIGH TEMP ALARM*************************************************************************************************************************************
                  case MENU_HIGH_TEMP_ALARM:
                    {
                      high_temp = true;
                      while (high_temp == true)
                      {
                        SetCursor (14, 3);
                        vga.print ("High Temp Alarm");
                        SetCursor (14, 22);
                        vga.print (set_high_alarm);
                        button = ReadButtons();
                        switch (button)
                        {
                          case BUTTON_UP:
                            {
                              set_high_alarm = set_high_alarm + .5;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_high_alarm);
                              delay(200);
                              break;
                            }
                          case BUTTON_DOWN:
                            {
                              set_high_alarm = set_high_alarm - .5;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_high_alarm);
                              delay(200);
                              break;
                            }
                          case BUTTON_SELECT:
                            {
                              SetCursor (13, 2);
                              vga.print ("                          ");
                              SetCursor (14, 22);
                              vga.print ("        ");
                              high_temp = false;
                              delay(200);
                              break;
                            }
                            break;
                        }
                      }
                      break;
                    }
                  //**************************************************************************************************************SET LOW TEMP ALARM*********************************************************************************************************************************
                  case MENU_LOW_TEMP_ALARM:
                    {
                      low_temp = true;
                      while (low_temp == true)
                      {
                        SetCursor (14, 3);
                        vga.print ("Low Temp Alarm");
                        SetCursor (14, 22);
                        vga.print (set_low_alarm);
                        button = ReadButtons();
                        switch (button)
                        {
                          case BUTTON_UP:
                            {
                              set_low_alarm = set_low_alarm + .5;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_low_alarm);
                              delay(200);
                              break;
                            }
                          case BUTTON_DOWN:
                            {
                              set_low_alarm = set_low_alarm - .5;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_low_alarm);
                              delay(200);
                              break;
                            }
                          case BUTTON_SELECT:
                            {
                              SetCursor (13, 2);
                              vga.print ("                          ");
                              SetCursor (14, 22);
                              vga.print ("        ");
                              low_temp = false;
                              delay(200);
                              break;
                            }
                            break;
                        }
                      }
                      break;
                    }
                  //******************************************************************************************************************SET Min Flow*************************************************************************************************************************************
                  case MENU_MIN_FLOW:
                    {
                      min_flow = true;
                      while (min_flow == true)
                      {
                        SetCursor (14, 3);
                        vga.print ("Set Min Flow");
                        SetCursor (14, 22);
                        vga.print (set_min_flow);
                        button = ReadButtons();
                        switch (button)
                        {
                          case BUTTON_UP:
                            {
                              set_min_flow = set_min_flow + 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_min_flow);
                              delay(200);
                              break;
                            }
                          case BUTTON_DOWN:
                            {
                              set_min_flow = set_min_flow - 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_min_flow);
                              delay(200);
                              break;
                            }
                          case BUTTON_SELECT:
                            {
                              SetCursor (13, 2);
                              vga.print ("                          ");
                              SetCursor (14, 22);
                              vga.print ("        ");
                              min_flow = false;
                              delay(200);
                              break;
                            }
                            break;
                        }
                      }
                      break;
                    }


                  //******************************************************************************************************************SET DOOR*************************************************************************************************************************************
                  case MENU_DOOR:
                    {
                      door = true;
                      while (door == true)
                      {
                        SetCursor (14, 3);
                        vga.print ("Door 1=0n 0=Off");
                        SetCursor (14, 22);
                        vga.print (set_door);
                        button = ReadButtons();
                        switch (button)
                        {
                          case BUTTON_UP:
                            {
                              set_door = set_door + 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_door);
                              delay(200);
                              break;
                            }
                          case BUTTON_DOWN:
                            {
                              set_door = set_door - 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_door);
                              delay(200);
                              break;
                            }
                          case BUTTON_SELECT:
                            {
                              SetCursor (13, 2);
                              vga.print ("                          ");
                              SetCursor (14, 22);
                              vga.print ("        ");
                              door = false;
                              delay(200);
                              break;
                            }
                            break;
                        }
                      }
                      break;
                    }

                  //******************************************************************************************************************SET BUZZER*************************************************************************************************************************************
                  case MENU_BUZZER:
                    {
                      buzzer = true;
                      while (buzzer == true)
                      {
                        SetCursor (14, 3);
                        vga.print ("Buzzer 1=On 0=Off");
                        SetCursor (14, 22);
                        vga.print (set_buzzer);
                        button = ReadButtons();
                        switch (button)
                        {
                          case BUTTON_UP:
                            {
                              set_buzzer = set_buzzer + 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_buzzer);
                              delay(200);
                              break;
                            }
                          case BUTTON_DOWN:
                            {
                              set_buzzer = set_buzzer - 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_buzzer);
                              delay(200);
                              break;
                            }
                          case BUTTON_SELECT:
                            {
                              SetCursor (13, 2);
                              vga.print ("                          ");
                              SetCursor (14, 22);
                              vga.print ("        ");
                              buzzer = false;
                              delay(200);
                              break;
                            }
                            break;
                        }
                      }
                      break;
                    }
                  //******************************************************************************************************************SET Alarms*************************************************************************************************************************************
                  case MENU_ALARMS:
                    {
                      alarms = true;
                      while (alarms == true)
                      {
                        SetCursor (14, 3);
                        vga.print ("Alarms 1=On 0=Off");
                        SetCursor (14, 22);
                        vga.print (set_alarms);
                        button = ReadButtons();
                        switch (button)
                        {
                          case BUTTON_UP:
                            {
                              set_alarms = set_alarms + 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_alarms);
                              delay(200);
                              break;
                            }
                          case BUTTON_DOWN:
                            {
                              set_alarms = set_alarms - 1;
                              SetCursor (14, 22);
                              vga.print ("        ");
                              SetCursor (14, 22);
                              vga.print (set_alarms);
                              delay(200);
                              break;
                            }
                          case BUTTON_SELECT:
                            {
                              SetCursor (13, 2);
                              vga.print ("                          ");
                              SetCursor (14, 22);
                              vga.print ("        ");
                              alarms = false;
                              delay(200);
                              break;
                            }
                            break;
                        }
                      }
                      break;
                    }
                  //******************************************************************************************************************EXIT MENU*************************************************************************************************************************************
                  case MENU_EXIT:
                    {
                      menu = false;
                      EEPROM.updateFloat(0, set_temperature);                //Upon menu exit, update these variables to eeprom so we dont loose them at power off or reset.
                      EEPROM.updateFloat(4, set_temp_swing);
                      EEPROM.updateInt(8, set_compressor_delay);
                      EEPROM.updateFloat(12, set_high_alarm);                //address 12
                      EEPROM.updateFloat(16, set_low_alarm);                 //address 16
                      EEPROM.updateInt(20, set_min_flow);                    //address 20
                      EEPROM.updateInt(24, set_door);                        //address 24
                      EEPROM.updateInt(28, set_buzzer);                      //address 28
                      EEPROM.updateInt(32, set_alarms);                      //address 32
                      delay(200);
                      break;
                    }
                    break;
                }
              }
              break;
          }
          SetColor (37, 40);
          SetCursor (13, 14);
          vga.print ("MENU");
          SetCursor (14, 3);
          vga.print (MENU[menu_number]);

        }
      }
      SetColor (37, 40);
      SetCursor (13, 1);
      vga.print("                   ");
      SetCursor (14, 1);
      vga.print("                     ");
      temp = millis() / 1000;

  }
  //************************************************************************************MENU EXIT******************************************************************************************************************************************************************
  // ***********************************************************************************NO LONGER IN MENU*********************************************************************************************************************************************************
  //vga display of Setpoints
  //
  SetColor(37, 40);
  SetCursor (10, 3);
  vga.print (set_temperature);
  SetCursor (10, 12);
  vga.print (set_temp_swing);
  SetCursor (10, 21);
  vga.print (set_compressor_delay);
  SetCursor (10, 25);
  vga.print (set_high_alarm);
  SetCursor (11, 4);
  vga.print (set_low_alarm);
  SetCursor (11, 11);
  vga.print (set_min_flow);
  SetCursor (11, 19);
  vga.print (set_door);
  SetCursor (11, 23);
  vga.print (set_buzzer);
  SetCursor (11, 28);
  vga.print (set_alarms);
  //
  //Read temperature for control
  //*****
  temptotal = 0;   //take "avgnum" of samples in order to take an average sensor reading, this helps with sensor accuracy.
  for (int x = 0; x < avgnum; x++) {
    tempReading = thermistor1.read();       //read temperature sensor
    temptotal = temptotal + tempReading;
  }
  spread = (set_temperature + set_temp_swing);
  tempavg = temptotal / avgnum;   //Should round to nearest .5 increment to get rid of jitter?  Maybe just do this on display?
  float temperatureC = (tempavg / 10); //
  //VGA Display Temp
  SetColor (37, 40);
  SetCursor(5, 2);
  vga.print("    ");
  SetCursor(5, 2);
  vga.print(temperatureC);
  //*************************
  // Read second temperature
  temptotal1 = 0;   //take "avgnum" of samples in order to take an average sensor reading, this helps with sensor accuracy.
  for (int g = 0; g < avgnum1; g++) {
    tempReading1 = thermistor2.read();       //read temperature sensor
    temptotal1 = temptotal1 + tempReading1;
  }
  tempavg1 = temptotal1 / avgnum1;   //Should round to nearest .5 increment to get rid of jitter?  Maybe just do this on display?
  float temperatureC1 = (tempavg1 / 10); //
  //VGA Display Temp
  SetColor (37, 40);
  SetCursor(5, 13);
  vga.print("    ");
  SetCursor(5, 13);
  vga.print(temperatureC1);
  //*************************
  //See if actual temperature is more than desired temperature accoring to threshold set by set_temp_swing
  //************
  if (temperatureC >= spread && a == LOW)
  {
    digitalWrite(COOL_PIN, HIGH);
    SetColor (30, 47);
    SetCursor (8, 2);
    vga.print (" ON ");
    delay(2000);                                      //Make sure compressor stays on for at least 2 seconds in case temperature fluctuates
    compressor_flag = true;
    a = HIGH;
    b = LOW;
    //Not here, need to figure out when turns off, not on
  }
  else if (temperatureC <= set_temperature && b == LOW)
  {
    SetColor (37, 40);
    SetCursor (8, 2);
    vga.print ("    ");
    digitalWrite(COOL_PIN, LOW);
    a = LOW;
    b = HIGH;
    if (compressor_flag == true)                                    //If the compressor was on, and just shut off, start timer for compressor delay.
    {
      compressor_time_temp = (millis() / 1000);
      compressor_flag = false;
    }
  }
  compressor_time = ((millis() / 1000) - compressor_time_temp) / 60; // See how long since compressor turned off in minutes
  timestamp = (millis() / 1000) - temp ;            //Turn off backlight after 100 seconds
  if (timestamp > 100 )
    //clear the buttonJustPressed or buttonJustReleased flags, they've already done their job now.
    if ( buttonJustPressed )
      buttonJustPressed = false;
  if ( buttonJustReleased )
    buttonJustReleased = false;
  //**********************
  // Read Flow meter on D2 using interupt. Flow sensor is something with a pulse out. Where there is at least a pulse per revolution. Hall Effect is the usual choose. Found mine on aliexpress.
  //**********************
  if ((millis() - oldTime) > 1000)   // Only process counters once per second
  {
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;

    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    unsigned int frac;
    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
  //Print Flow Rate from above code
  //VGA Display Flow
  SetColor (37, 40);
  SetCursor(5, 26);
  vga.print("    ");
  SetCursor(5, 26);
  vga.print(int(flowRate));
  tempAlarm = temperatureC;
  systctl();

}
//********************************************************************************************************END OF PROGRAM, Below are functions used above***********************************************************************************************************
/*--------------------------------------------------------------------------------------
  ReadButtons()
  Detect the button pressed and return the value
  Uses global values buttonWas, buttonJustPressed, buttonJustReleased.
  --------------------------------------------------------------------------------------*/
byte ReadButtons()
{
  unsigned int buttonVoltage;
  byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn
  delay (50);
  //read the button ADC pin voltage
  buttonVoltage = analogRead( BUTTON_ADC_PIN );
  //sense if the voltage falls within valid voltage windows
  if ( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_RIGHT;
  }
  else if (   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
              && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_UP;
  }
  else if (   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
              && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_DOWN;
  }
  else if (   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
              && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_LEFT;
  }
  else if (   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
              && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_SELECT;
  }
  //handle button flags for just pressed and just released events
  if ( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
  {
    //the button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event
    //it's the duty of the receiver to clear these flags if it wants to detect a new button change event
    buttonJustPressed  = true;
    buttonJustReleased = false;
  }
  if ( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
  {
    buttonJustPressed  = false;
    buttonJustReleased = true;
  }
  //save the latest button value, for change event detection next time round
  buttonWas = button;
  return ( button );
}
//*****************up pulse count****************
void pulseCounter()//Part of the flow meter
{
  // Increment the pulse counter
  pulseCount++;
}

void systctl()// this is where it looks up flags for door alarm and flow to control a relay output
{
  //Flow Rate Alarm
  if (set_min_flow >= flowRate)
  {
    SetColor (30, 47);
    SetCursor (8, 11);
    vga.print ("**");
    mIn = 1;
  }
  else {
    SetColor (37, 40);
    SetCursor (8, 11);
    vga.print ("    ");
    mIn = 0;
  }
  //*****************
  //Door Alarm
  //read the door switch value into a variable. Input requires a 10k-20k to +5vdc.
  int doOr = digitalRead(dOorIn);
  if (doOr == 1)
  {
    SetColor (30, 47);
    SetCursor (8, 26);
    vga.print ("**");
  }
  else {
    SetColor (37, 40);
    SetCursor (8, 26);
    vga.print ("    ");
  }
  //Temperature alarm
  if ((tempAlarm >= set_high_alarm) || (tempAlarm <= set_low_alarm)) //compare temperature to high and low alarm set point and set values required.
  {
    SetColor (30, 47);
    SetCursor (8, 19);
    vga.print ("**");
    hiGh = 1;
  }
  else {
    SetColor (37, 40);
    SetCursor (8, 19);
    vga.print ("    ");
    hiGh = 0;
  }
  //Alarm comparison to run relay output
  if (((hiGh == 1) || (mIn == 1) || ((doOr == 1) && (set_door == 1))) && (set_alarms == 1))
  {
    SetColor (30, 47);
    SetCursor (1, 3);
    vga.print ("                         ");
    SetCursor (1, 12);
    vga.print (" ALARM ");
    SetColor (37, 40);
    alaRms = 1;
    digitalWrite(alarm_PIN, HIGH);
    if (set_buzzer == 1) {
      digitalWrite(buzzer_PIN, HIGH);
    }
    else digitalWrite(buzzer_PIN, LOW);
  }
  else
  {
    SetColor (37, 40);
    SetCursor (1, 2);
    vga.print (" IceMaker Controller V6.1 ");
    digitalWrite(alarm_PIN, LOW);
    alaRms = 0;
    digitalWrite(buzzer_PIN, LOW);
  }
}

//VGA Display Stuff
void SetNoWrap() {
  vga.write (ESC, 2);
  vga.write ("?7l");
}

void SetColor (int fore, int back) {
  vga.write (ESC, 2);
  vga.print (fore);
  vga.write (";");
  vga.print (back);
  vga.write ("m");
}

void SetCursor (int x, int y) {
  vga.write (ESC, 2);
  vga.print (x);
  vga.write (";");
  vga.print (y);
  vga.write ("H");
}

void InitScreen() {
  // Draw initial screen content.
  vga.write(CLR, 4);
  //draw box
  SetCursor (0, 0);
  SetColor (37, 40);
  vga.write(UPBOX, 32); // Create a box on the UI
  //for (int f = 1; f < 2; f++)
  // {
  //   SetCursor (f, 0);
  SetCursor (1, 0);
  vga.write(MIDBOX, 32);
  // }
  SetCursor (2, 0);
  vga.write(CROSSBOX, 32);
  for (int f = 3; f < 6; f++)
  {
    SetCursor (f, 0);
    vga.write(MIDBOX, 32);
  }
  SetCursor (6, 0);
  vga.write(CROSSBOX, 32);
  for (int f = 7; f < 12; f++)
  {
    SetCursor (f, 0);
    vga.write(MIDBOX, 32);
  }
  SetCursor (12, 0);
  vga.write(CROSSBOX, 32);
  for (int f = 13; f < 15; f++)
  {
    SetCursor (f, 0);
    vga.write(MIDBOX, 32);
  }
  SetCursor (15, 0);
  vga.write(LOWBOX, 32);
  //TEXT
  SetCursor (1, 2);  // some text and stuff
  SetColor (37, 40);
  vga.print (" IceMaker Controller V6.1 ");
  SetCursor (3, 1);
  vga.print ("I/P Temp");
  SetCursor (5, 3);
  vga.print ("0");
  SetCursor (3, 12);
  vga.print ("O/P Temp");
  SetCursor (5, 14);
  vga.print ("0");
  SetCursor (3, 25);
  vga.print ("Flow");
  SetCursor (5, 26);
  vga.print ("0");
  SetCursor (7, 2);
  vga.print ("Comp");
  // SetColor (30, 47);
  // SetCursor (8, 2);
  //vga.print (" ON ");
  SetColor (37, 40);
  SetCursor (13, 14);
  vga.print ("MENU");
  SetCursor (7, 10);
  vga.print ("Flow");
  SetCursor (7, 18);
  vga.print ("Temp");
  SetCursor (7, 25);
  vga.print ("Door");
  //Setpoints
  SetCursor (10, 1);
  vga.print ("T:");
  SetCursor (10, 8);
  vga.print ("TS:");
  SetCursor (10, 17);
  vga.print ("CD:");
  SetCursor (10, 22);
  vga.print ("TH:");
  SetCursor (11, 1);
  vga.print ("TL:");
  SetCursor (11, 9);
  vga.print ("F:");
  SetCursor (11, 17);
  vga.print ("D:");
  SetCursor (11, 21);
  vga.print ("B:");
  SetCursor (11, 26);
  vga.print ("A:");

}
