/*

 Copyright (c) 2014.  All rights reserved.
 An Open Source Arduino based RGB LED driver for Quadcopters.
 Supports MAVLink 1.0 message decoding and RGB pattern flasiing,  
 FrSky Telemetry translation, using an Arduino Pro Mini 5v 16MHz Board
 
 Program  : MavLED
 Version  : V 0.01 Dec 14, 2014
 Author   : jjstecchino
 
 Based on IO-Board by Jani Hirvinen, Sandro Beningo (MAVLink routines), 
 Mike Smith (BetterStream and Fast Serial libraries), FastLED library
 
 This program is free software: you can redistribute it and/or modify,
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>
 
 */

/*
//////////////////////////////////////////////////////////////////////////
 //  Description: 
 // 
 //  This is an Arduino sketch to drive 4 arrays of RGB LEDs, using an
 //  Arduino Pro Mini. The software listens to MAVLink messages and 
 //  canges patterns accordingly.
 //  Supports:  LPD8806 based RGB LED Arrays
 //
 //  If you use, redistribute this please mention original source.
 //
 //  Arduino Pro Mini pinouts and connections:
 //
 //                         G R  
 //         D D D D D D D D N S R T
 //         9 8 7 6 5 4 3 2 D T X X
 //         | | | | | | | | | | | |
 //      +----------------------------+
 //      |O O O O O O O O O O O O O   |
 //  V - |O                          O| _ GRN 
 //  G - |O           A A            O| - RX  F
 //      |            4 5            O| - TX  T
 // A7 - |O           | |            O| - VCC D
 // A6 - |O           O O            O| _ GND I
 //      |O O O O O O O O   O O O O  O| - BLK
 //      +----------------------------+
 //       | | | | | | | |   | | | |
 //       D D D D A A A A   V R G R
 //       1 1 1 1 0 1 2 3   C S N A
 //       0 1 2 3           C T D W
 //
 // More information, check 
 //
/* **************************************************************************** */

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------
#include <FastSerial.h>       // Fast serial library
#include <SoftwareSerial.h>
#include "FastLED.h"          // Library to drive RGB Leds
#include <EEPROM.h>
#include <GCS_MAVLink.h>
#include <AP_Common.h>
#include <AP_Math.h>
#include <math.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <SimpleTimer.h>

// Configurations
#include "MavLED_Pattern.h"  // LED strips configuration and pre-defined patterns
#include "MavLED_Board.h"
#include "MavLED_Eeprom.h"

// Get the common arduino functions
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "wiring.h"
#endif



// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------
#define CHKVER 40      // EEPROM checknum
#define SER_TX 10      // Pin for serial Tx
#define SER_RX 11      // Pin for serial Rx
#define LED_PIN 13     // Heartbeat LED
#define RESET_PIN 13   // Reset to factory default. Connect this pin to ground to force a reset
#define LOOPTIME  50   // Main loop time for heartbeat
#define TELEMETRY_SPEED  57600  // How fast our MAVLink telemetry is coming to Serial port

// define DBSER to use serial port for debug.
// comment it out to use serial for FrSky telemetry
// same SER_RX and SER_TX pins will be used 
// for frsky and debug serial
//#define DBSER  // comment this to disable debug and start FrSky telemetry
#ifndef DBSER
#define FRSER  // Serial port used for FrSky Telemetry unless DBSER defined
#endif


// Macros
#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 
#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];}))

#ifdef membug
#include <MemoryFree.h>
#endif

// ----------------------------------------------------------------------------
// Global Variables and Classes 
// ----------------------------------------------------------------------------
static byte LeRiPatt = NOMAVLINK; // default pattern is full ON

static long p_preMillis;
static long p_curMillis;
static int p_delMillis = LOOPTIME;

static int curPwm;
static int prePwm;

int messageCounter;

static bool mavlink_active;
byte hbStatus;

byte voltAlarm;  // Alarm holder for internal voltage alarms, trigger 4 volts

float boardVoltage;
int i2cErrorCount;

byte ledState;
byte baseState;  // Bit mask for different basic output LEDs like so called Left/Right 

CRGB leds[NUM_ARMS][NUM_LEDS];  // Define the array of leds
FastSerialPort0(Serial);        // Fast serial port for mavlink
SimpleTimer  mavlinkTimer;      // mavlink timer

#ifdef FRSER
SoftwareSerial frSerial(SER_RX, SER_TX,true);
#endif

#ifdef DBSER    // if debug mode create dbSerial and setup print macros
SoftwareSerial dbSerial(SER_RX, SER_TX);
#define DPL if(debug) dbSerial.println
#define DPN if(debug) dbSerial.print
byte debug = 1;
#else
#define DPL if(debug) {}
#define DPN if(debug) {}
byte debug = 0;
#endif


// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

void setup() { 
  
  // Setup led strips and turn leds off
  FastLED.addLeds<CHIPSET, LF_DPIN, LF_CPIN, LED_ORDER>(leds[0], NUM_LEDS);
  FastLED.addLeds<CHIPSET, RF_DPIN, RF_CPIN, LED_ORDER>(leds[1], NUM_LEDS);
  FastLED.addLeds<CHIPSET, LB_DPIN, LB_CPIN, LED_ORDER>(leds[2], NUM_LEDS);
  FastLED.addLeds<CHIPSET, RB_DPIN, RB_CPIN, LED_ORDER>(leds[3], NUM_LEDS);
  showPattern(NONE);

  // Initialize mavlink fastserial, speed
  Serial.begin(TELEMETRY_SPEED);

  // if debug mode start debug serial port
#ifdef SERDB  
  dbSerial.begin(38400);  // 38400 is just the right speed
  DPL("Debug port Open"); // print start debug messages
  DPL("No input from this serialport.  ");
#endif

  // otherwise start FrSky serial port
#ifdef FRSER
  frSerial.begin(9600);  // FrSky speed is 9600 bps
#endif

  // Check if factory reset requested by grounding pin 13
  digitalWrite(RESET_PIN, HIGH);      // Let's put PULLUP high to avoid accidental erases
  if(digitalRead(RESET_PIN) == 0) {    
    DPL("Force erase pin LOW, Erasing EEPROM");
    DPN("Writing EEPROM...");
    writeFactorySettings();
    DPL(" done.");
  }

  // Check that EEPROM has initial settings, if not write them
  if(readEEPROM(CHK1) + readEEPROM(CHK2) != CHKVER) {
    // Write factory settings on EEPROM
    DPN("Writing EEPROM...");
    writeFactorySettings();
    DPL(" done.");
  }

#ifdef DUMPEEPROM
  // For debug needs, should never be activated on real-life
  DPN("Eeprom check 1: ");
  DPN(CHK1);
  DPN("Eeprom check 2: ");
  DPN(CHK1);
  DPN("Eeprom version: ");
  DPN(CHKVER);
  for(int edump = 0; edump <= 24; edump ++) {
    DPN("EEPROM SLOT: ");
    DPN(edump);
    DPN(" VALUE: ");
    DPL(readEEPROM(edump));     
  }

  // For debug needs, should never be activated on real-life
  for(int edump = 60; edump <= 80; edump ++) {
    DPN("EEPROM SLOT: ");
    DPN(edump);
    DPN(" VALUE: ");
    DPL(readEEPROM(edump));     
  }
#endif

  // Read most important values from EEPROM to their variables  
  LEFT = readEEPROM(LEFT_IO_ADDR);
  RIGHT = readEEPROM(RIGHT_IO_ADDR);
  FRONT = readEEPROM(FRONT_IO_ADDR);
  REAR = readEEPROM(REAR_IO_ADDR);
  ledPin = readEEPROM(LEDPIN_IO_ADDR);

  // setup mavlink port
  mavlink_comm_0_port = &Serial;

}

// ----------------------------------------------------------------------------
// Main Loop
// ----------------------------------------------------------------------------

void loop() { 
  showPattern(PULSE); 
}








