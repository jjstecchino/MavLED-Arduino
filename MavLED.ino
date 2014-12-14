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
//#include "IOBoard.h"
//#include "IOEEPROM.h"

// Get the common arduino functions
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "wiring.h"
#endif



// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------

// Macros
#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 
#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];}))

// RGB Led array
#define CHIPSET LPD8806
#define LED_ORDER BRG
#define LF_DPIN 2
#define LF_CPIN 3
#define RF_DPIN 4
#define RF_CPIN 5
#define LB_DPIN 6
#define LB_CPIN 7
#define RB_DPIN 8
#define RB_CPIN 9
#define BACKGROUND Blue
#define FOREGROUND Red
#define NUM_LEDS 8
#define NUM_ARMS 4

#ifdef membug
#include <MemoryFree.h>
#endif

// ----------------------------------------------------------------------------
// Global Variables
// ----------------------------------------------------------------------------

CRGB leds[NUM_ARMS][NUM_LEDS];  // Define the array of leds

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

void setup() { 
  FastLED.addLeds<CHIPSET, LF_DPIN, LF_CPIN, LED_ORDER>(leds[0], NUM_LEDS);
  FastLED.addLeds<CHIPSET, RF_DPIN, RF_CPIN, LED_ORDER>(leds[1], NUM_LEDS);
  FastLED.addLeds<CHIPSET, LB_DPIN, LB_CPIN, LED_ORDER>(leds[2], NUM_LEDS);
  FastLED.addLeds<CHIPSET, RB_DPIN, RB_CPIN, LED_ORDER>(leds[3], NUM_LEDS);

  for(int i = 0; i < NUM_ARMS; i++) {
    for(int j = 0; j < NUM_LEDS; j++) {
      leds[i][j] = CRGB::BACKGROUND;
    }
  }
}

// ----------------------------------------------------------------------------
// Main Loop
// ----------------------------------------------------------------------------

void loop() { 
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    for(int j = 0; j < NUM_ARMS; j++) {
      leds[j][i] = CRGB::FOREGROUND;
    }
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    for(int j = 0; j < NUM_ARMS; j++) {
      leds[j][i] = CRGB::BACKGROUND;
    }
    // Wait a little bit before we loop around and do it again
    delay(30);
  }

  // Now go in the other direction.  
  for(int i = NUM_LEDS-1; i >= 0; i--) {
    // Set the i'th led to red 
    for(int j = 0; j < NUM_ARMS; j++) {
      leds[j][i] = CRGB::FOREGROUND;
    }
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    for(int j = 0; j < NUM_ARMS; j++) {
      leds[j][i] = CRGB::BACKGROUND;
    }
    // Wait a little bit before we loop around and do it again
    delay(30);
  } 
}





