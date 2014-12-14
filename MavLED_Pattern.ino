/************************************************************************

 Copyright (c) 2014.  All rights reserved.
 An Open Source Arduino based RGB LED driver for Quadcopters.
 Supports MAVLink 1.0 message decoding and RGB pattern flasiing,  
 FrSky Telemetry translation, using an Arduino Pro Mini 5v 16MHz Board
 
 Program  : MavLED_Pattern
 Version  : V 0.01 Dec 14, 2014
 Author   : jjstecchino
 
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
 
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//  Description: 
// 
//  Functions to show different patterns 
/////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------
// Show specified pattern
// -------------------------------------------------------------
void showPattern(int pattern) {
  switch (pattern) {
  case NONE: 
    {
      patternNone();
      break;
    }
  case CYLON: 
    {
      patternCylon(CRGB::FOREGROUND, CRGB::BACKGROUND);
      break;
    }
  case PULSE: 
    {
      patternPulse(CRGB::FOREGROUND, SLOW_PULSE);
      break;
    }
  default: 
    {
      patternNone();
    }
  }
}

// -------------------------------------------------------------
// Show pulse pattern
// -------------------------------------------------------------
void patternPulse(CRGB foreground, int msDelay) {
    // Set the i'th led to foreground color
  for(int i = 0; i < NUM_LEDS; i++) {
    // do it for avery arm
    for(int j = 0; j < NUM_ARMS; j++) {
      // Set the i'th led to foreground color
      leds[j][i] = foreground;
    }
    // Show the leds
    FastLED.show();
  }
    //progressively increase brightness
    for(int i=30; i<255; i++) {
      FastLED.setBrightness(i);
      FastLED.show();
      delay(msDelay);
    }
    //now decrease increase
    for(int i=254; i>30; i--) {
      FastLED.setBrightness(i);
      FastLED.show();
      delay(msDelay);
    }
  
}


// -------------------------------------------------------------
// Show cylon pattern
// -------------------------------------------------------------
void patternCylon(CRGB foreground, CRGB background) {
  // First slide the led in one direction
  // Set the i'th led to foreground color
  for(int i = 0; i < NUM_LEDS; i++) {
    // do it for avery arm
    for(int j = 0; j < NUM_ARMS; j++) {
      // Set the i'th led to foreground color
      leds[j][i] = foreground;
    }
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    for(int j = 0; j < NUM_ARMS; j++) {
      leds[j][i] = background;
    }
    // Wait a little bit before we loop around and do it again
    delay(30);
  }

  // Now go in the other direction.  
  // Set the i'th led to foreground color
  for(int i = NUM_LEDS-1; i >= 0; i--) {
    // do it for avery arm
    for(int j = 0; j < NUM_ARMS; j++) {
      leds[j][i] = foreground;
    }
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    for(int j = 0; j < NUM_ARMS; j++) {
      leds[j][i] = background;
    }
    // Wait a little bit before we loop around and do it again
    delay(30);
  } 
}

// -------------------------------------------------------------
// Turn off all leds
// -------------------------------------------------------------
void patternNone() {
  for(int i = 0; i < NUM_LEDS; i++) {
    for(int j = 0; j < NUM_ARMS; j++) {
      leds[j][i] = CRGB::Black;
    }
    // Show the leds
    FastLED.show();
  }
}
