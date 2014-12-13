/*

 Copyright (c) 2014.  All rights reserved.
 An Open Source Arduino based RGB LED driver for Quadcopters.
 Supports MAVLink 1.0 message decoding and RGB pattern flasiing,  
 FrSky Telemetry translation, using an Arduino Pro Mini 5v 16MHz Board
 
 Program  : MavLED
 Version  : V 0.01 Dec 14, 2014
 Author   : Claudio Guareschi
 
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
//             S M M G       R T R
//         5 5 C O I N D D D X X S
//         V V K S S D 7 6 5 1 1 T
//         | | | | | | | | | | | |
//      +----------------------------+
//      |O O O O O O O O O O O O O   |
// O1 - |O O   | | |                O| _ DTS 
// O2 - |O O   3 2 1                O| - RX  F
// O3 - |O O   1 1 1                O| - TX  T
// O4 - |O O   D D D                O| - 5V  D
// O5 - |O O                        O| _ CTS I
// O6 - |O O O O O O O O   O O O O  O| - GND
//      +----------------------------+
//       |   | | | | | |   | | | |
//       C   G 5 A A A A   S S 5 G
//       O   N V 0 1 2 3   D C V N
//       M   D             A L   D
//
// More information, check 
//
/* **************************************************************************** */

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
#define DATA_PIN 9
#define CLOCK_PIN 8
#define BACKGROUND Brown
#define FOREGROUND Red
#define NUM_LEDS 8

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------

#include "FastLED.h"          // Library to drive RGB Leds
#include <FastSerial.h>       // Fast serial library
#include <AP_Common.h>
#include <AP_Math.h>
#include <math.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <SimpleTimer.h>
#include <GCS_MAVLink.h>
#include <SoftwareSerial.h>

#ifdef membug
#include <MemoryFree.h>
#endif


// Configurations
#include "IOBoard.h"
#include "IOEEPROM.h"


// Get the common arduino functions
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "wiring.h"
#endif





// ----------------------------------------------------------------------------
// Global Variables
// ----------------------------------------------------------------------------

CRGB leds[NUM_LEDS];  // Define the array of leds

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

void setup() { 
	FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, LED_ORDER>(leds, NUM_LEDS);
        for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = CRGB::BACKGROUND;
        }
 }

// ----------------------------------------------------------------------------
// Main Loop
// ----------------------------------------------------------------------------

void loop() { 
	// First slide the led in one direction
	for(int i = 0; i < NUM_LEDS; i++) {
		// Set the i'th led to red 
		leds[i] = CRGB::FOREGROUND;
		// Show the leds
		FastLED.show();
		// now that we've shown the leds, reset the i'th led to black
		leds[i] = CRGB::BACKGROUND;
		// Wait a little bit before we loop around and do it again
		delay(30);
	}

	// Now go in the other direction.  
	for(int i = NUM_LEDS-1; i >= 0; i--) {
		// Set the i'th led to red 
		leds[i] = CRGB::FOREGROUND;
		// Show the leds
		FastLED.show();
		// now that we've shown the leds, reset the i'th led to black
		leds[i] = CRGB::BACKGROUND;
		// Wait a little bit before we loop around and do it again
		delay(30);
	} 
}




