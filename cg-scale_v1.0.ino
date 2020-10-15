/*
By: Thomas KRISTNER
Date: May 2020

0-6Kg Aeromodelism CG scale based on Sparfun hardware.
Max weight depend of the loadcells used and the 3D printed parts strengh, actual design is supporting 0 to 6 Kg.

2x Seeed LOAD CELL ( YZC-1B ) 0-3KG (if another loadcells fit any 4 wire analog loadcell is ok) please solder the loadcell foiled mass to avoid noise in analog signal.
2x Sparkfun QWIIC SCALE - NAU7802
1x SparkFun Thing Plus - ESP32 WROOM
1x SparkFun Qwiic Mux Breakout - 8 Channel (TCA9548A)
1x Button
1x Sparkfun MICRO OLED BREAKOUT QWIIC
1x LiPo 1S 3.7V ~750mAh to make it portable (recharged through the SparkFun Thing Plus usb connection)
1x Switch placed between board LiPo port and the LiPo battery. This allow to power up the scale when on battery or to charge battery when connected to Sparkfun USB)
*/

#include"Arduino.h"
#include <Wire.h>
#include <SFE_MicroOLED.h>
#include <SimpleTimer.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"

#define WINDOW_SIZE 3

/*
SFE_MicroOLED library assumes a reset pin is necessary. The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is not being used
DC_JUMPER is the MicroOLED I2C Address Select jumper. Set to 1 if the jumper is open (Default), or set to 0 if it's closed.
*/
#define PIN_RESET 9
#define DC_JUMPER 1

/*
Simple timer instance
*/
SimpleTimer timer0;

/*
NAU7802 class for the front and back cells
*/
NAU7802 Scale_front_cell;
NAU7802 Scale_back_cell;

/*
MicroOLED Object creation
*/
MicroOLED oled(PIN_RESET, DC_JUMPER);

/*
Define pushbutton pin connected to port x
Initialize buttonState and previousbuttonState to  0
i is a counter for the selection of the CG peg distance array 110,135,160,185,210,235 mm
*/
const int buttonPin = 13;
int buttonState = 0;
int previousbuttonState = 0;
int i = 0;

/*
Value used to convert the load cell reading to lbs or kg, this is mandatory.
You have to use calibrateScale() for example by running the example "Example2_CompleteScale" of the SparkFun_Qwiic_Scale_NAU7802_Arduino_Library, then send "c" in the serial monitor to start with calibration.
*/
const float Front_settingCalibrationFactor = 1395.60;
const float Back_settingCalibrationFactor = 1466.10;



long Front_settingZeroOffset;
long Back_settingZeroOffset;
int front_currentWeight = 0;
int front_INDEX = 0;
int front_VALUE = 0;
int front_SUM = 0;
int front_READINGS[WINDOW_SIZE];
int front_avgWeight = 0;
int back_currentWeight;
int back_INDEX = 0;
int back_VALUE = 0;
int back_SUM = 0;
int back_READINGS[WINDOW_SIZE];
int back_avgWeight = 0;
int total_weight = 0;
float CG_ratio = 0.0;
int CG = 0;

/*
Populating the array of wing peg real distances (if you change the 3D model please update these values in mm).
LEstopperDist is the value in mm of the projected distance of the front wing peg point to stopper pin
*/
const int WingPegDist_array [6] = {110,135,160,185,210,235};
const int LEstopperDist = 40;
int WingPegDist = WingPegDist_array[5];

/*
The button allows you to step through the values
*/
void button_WingPegDist()
{
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH)
  {
      if (previousbuttonState == LOW)
    {
      WingPegDist = WingPegDist_array[i];
      i+=1;
    }
  }
  if (i==6)
  {
    i=0;
  }
  previousbuttonState = buttonState;
}


void read_front_back_total_cg()
{
  enableMuxPort(0);      //Tell mux to connect to port 0, and this port only
  if (Scale_front_cell.available() == true)
  {
    long front_currentReading = Scale_front_cell.getReading();
    float front_float_currentWeight = Scale_front_cell.getWeight();
    front_currentWeight = int(front_float_currentWeight);
    front_SUM = front_SUM - front_READINGS[front_INDEX];        // Remove the oldest entry from the sum
    front_VALUE = front_currentWeight;                          // Read the next sensor value
    front_READINGS[front_INDEX] = front_VALUE;                  // Add the newest reading to the window
    front_SUM = front_SUM + front_VALUE;                        // Add the newest reading to the sum
    front_INDEX = (front_INDEX+1) % WINDOW_SIZE;                // Increment the index, and wrap to 0 if it exceeds the window size
    front_avgWeight = front_SUM / WINDOW_SIZE;                  // Divide the sum of the window by the window size for the result
  }
  disableMuxPort(0);     // Tell mux to disconnect from port 0
  enableMuxPort(1);
  if (Scale_back_cell.available() == true)
  {
    long back_currentReading = Scale_back_cell.getReading();
    float back_float_currentWeight = Scale_back_cell.getWeight();
    back_currentWeight = int(back_float_currentWeight);
    back_SUM = back_SUM - back_READINGS[back_INDEX];
    back_VALUE = back_currentWeight;
    back_READINGS[back_INDEX] = back_VALUE;
    back_SUM = back_SUM + back_VALUE;
    back_INDEX = (back_INDEX+1) % WINDOW_SIZE;
    back_avgWeight = back_SUM / WINDOW_SIZE;
  }
  disableMuxPort(1);
  total_weight = front_avgWeight + back_avgWeight;
  if (front_avgWeight >  0 || back_avgWeight > 0)
  {
    CG_ratio = float(back_avgWeight) / float(total_weight);
    CG = (float(WingPegDist) * CG_ratio) + float(LEstopperDist);
  }
  else{
    CG_ratio = 0.0;
    CG = 0;
  }
  /*
  Serial.println("Printing values for debug ");
  Serial.print("Wing peg distance (mm) selected: ");
  Serial.println(WingPegDist);
  Serial.print("Stopper distance (mm): ");
  Serial.println(LEstopperDist);
  Serial.print("Measured front weight (g): ");
  Serial.println(front_avgWeight);
  Serial.print("Measured back weight (g): ");
  Serial.println(back_avgWeight);
  Serial.print("Measured total weight (g): ");
  Serial.println(total_weight);
  Serial.print("CG ratio: ");
  Serial.println(CG_ratio);
  Serial.print("Calculated CG distance (mm): ");
  Serial.println(CG);
  */
}

/*
oled_display define the whole OLED display layout
*/
void oled_display()
{
  enableMuxPort(7);
  oled.clear(PAGE);            // Clear the display
  oled.setCursor(0, 0);        // Set cursor to the top-left of the display
  oled.setFontType(0);         // choose the smallest font
  oled.print("<-> ");
  oled.setFontType(0);
  oled.print(WingPegDist);
  oled.setFontType(0);
  oled.print("mm");
  oled.setCursor(0, 8);
  oled.setFontType(0);
  oled.print("F: ");
  oled.setFontType(0);
  oled.print(front_avgWeight);
  oled.setCursor(0, 16);
  oled.setFontType(0);
  oled.print("B: ");
  oled.setFontType(0);
  oled.print(back_avgWeight);
  oled.setCursor(0, 24);
  oled.setFontType(0);
  oled.print("T: ");
  oled.setFontType(0);
  oled.print(total_weight);
  oled.setCursor(0, 32);
  oled.setFontType(0);
  oled.print("CG ");
  oled.setFontType(0);
  oled.print(CG);
  oled.setFontType(0);
  oled.print("mm");
  oled.display();
  disableMuxPort(7);
}


void setup()
{
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);      //Qwiic Scale is capable of running at 400kHz I2C
  pinMode(buttonPin, INPUT);
  /*
  OLED display section
  */
  enableMuxPort(7);
  oled.begin();               // Initialize the OLED
  oled.clear(ALL);            // Clear the display's internal memory
  oled.display();             // Display what's in the buffer (splashscreen)
  oled.clear(PAGE);           // Clear the buffer.
  disableMuxPort(7);

  /*
  Front cell setup
  */
  enableMuxPort(0);
  while (Scale_front_cell.begin() == false)
  {
    delay(100);
    Serial.print("front cell issue");
  }
  Scale_front_cell.setSampleRate(NAU7802_SPS_320);            // Increase to max sample rate
  Scale_front_cell.calibrateAFE();                            // Re-calibrate analog front end when we change gain, sample rate, or channel
  /*
  configuring the zero offset
  */
  Scale_front_cell.calculateZeroOffset(64);                   //Zeroing the scale (Tare). with an average over 64 readings (even 64 is pretty quick).
  Front_settingZeroOffset = Scale_front_cell.getZeroOffset();
  Scale_front_cell.setZeroOffset(Front_settingZeroOffset);
  /*
  configuring calibration factor
  */
  Scale_front_cell.setCalibrationFactor(Front_settingCalibrationFactor);
  disableMuxPort(0);

  /*
  Back cell setup
  */
  enableMuxPort(1);
  while (Scale_back_cell.begin() == false)
  {
    delay(100);
    Serial.print("back cell issue");
  }
  Scale_back_cell.setSampleRate(NAU7802_SPS_320);
  Scale_back_cell.calibrateAFE();
  /*
  configuring the zero offset
  */
  Scale_back_cell.calculateZeroOffset(64);
  Back_settingZeroOffset = Scale_back_cell.getZeroOffset();
  Scale_back_cell.setZeroOffset(Back_settingZeroOffset);
  /*
  configuring calibration factor
  */
  Scale_back_cell.setCalibrationFactor(Back_settingCalibrationFactor);
  disableMuxPort(1);
  /*
  SimpleTimer setup (this is to manage code execution without using delay elsewhere than in the setup part)
  setInterval takes milliseconds.
  */
  timer0.setInterval(200, read_front_back_total_cg);
  timer0.setInterval(500, oled_display);
  timer0.setInterval(100, button_WingPegDist);
}

/*
Execute SimpleTimer instances in loop.
*/
void loop()
{
  timer0.run();
}
