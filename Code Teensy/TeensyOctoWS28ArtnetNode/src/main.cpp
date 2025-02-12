/**
 * @file main.cpp
 * @brief Teensy OctoWS28 Artnet Node
 * @details An Artnet node built with Teensy 4.1 + OctoWS28
 * @version V0.2.1
 * @date 2023
 * @author Clément SAILLANT
 * @author maigre
 * @author - Studio Jordan Shaw
 *
 * @copyright GNU General Public License v3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Debug.h"
#include "TeensyID.h"
#include "gamma8.h"
#include <Artnet.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <OctoWS2811.h>
#include <SPI.h>

// Set to 1 to enable debug output
const int debug_set = 1;

// Set to 0 to disable Artnet and run a test pattern or 1 to enable Artnet
const int artnet_set = 1;
// To help with logevity, brightness is set to ~50% (0-255) = 30A in full white
const int BRIGHTNESS = 200;
const int blackout_pin = 40;
/*
 COLOR_CORRECTION
   TypicalLEDStrip=    0xFFB0F0     255, 176, 240
   Typical8mmPixel=    0xFFE08C     255, 224, 140
   TypicalPixelString= 0xFFE08C     255, 224, 140
   UncorrectedColor=   0xFFFFFF     255, 255, 255
 */
// #define COLOR_CORRECTION TypicalLEDStrip
const int COLOR_CORRECTION = UncorrectedColor;

// Throttling refresh for when using 24+ universes
// ie. 510 leds / 3 universes per pin
const int FRAMES_PER_SECOND = 30;

// CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet
// first universe as 0.
const int startUniverse = 0;

// Network IP addresses
byte ip[] = {2, 12, 0, 254}; // IP address of the node (254 is default, will be
                             // overwriten by ID_ETENDARD read from EEPROM)

// Etendard V0.1 (GRAZ)
#if V_ETENDARD == 0
const int numPins = 18; // Number of pins used for LED output = 32
const byte pinList[numPins] = {
    30, 29, 28, 27, 26, 25, 24, 12, 11,
    31, 32, 33, 34, 35, 36, 37, 38, 39}; // List of pins used for LED output
const int Led_for_one_strip = 108;       // Number of LEDs per strip
const int Nb_string_strip = 2;           // Number of strips per pin
#endif

// Etendard V1.a (KXKM)
#if V_ETENDARD == 1
const int numPins = 18; // Number of pins used for LED output = 32
const byte pinList[numPins] = {
    33, 32, 31, 30, 29, 28, 27, 26, 25,
    24, 12, 11, 10, 9,  8,  7,  6,  5}; // List of pins used for LED output
const int Led_for_one_strip = 138;      // Number of LEDs per strip
const int Nb_string_strip = 2;          // Number of strips per pin
#endif

// Etendard V2.a (KXKM) version with 1 strip of 138 LEDs per pin in 36 output
// pins
#if V_ETENDARD == 2
const int numPins = 36;
const byte pinList[numPins] = {23, 22, 21, 20, 18, 17, 16, 15, 14, 13,
                               41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
                               31, 30, 29, 28, 27, 26, 25, 24, 12, 11,
                               10, 9,  8,  7,  6,  5}; // List of pins used for
                                                       // LED output
const int Led_for_one_strip = 138; // Number of LEDs per strip
const int Nb_string_strip = 1;     // Number of strips per pin
#endif

// calculate the number of leds per strip and the total number of leds in the
// system
const int ledsPerStrip = Led_for_one_strip * Nb_string_strip;
const byte numStrips = numPins;
const int numLeds = ledsPerStrip * numStrips;
const int numberOfChannels = numLeds * 3;

// Define your FastLED pixels
CRGB rgbarray[numPins * ledsPerStrip];

// Memory buffer to artnet data
/* These buffers need to be large enough for all the pixels.
 The total number of pixels is "ledsPerStrip * numPins".
 Each pixel needs 3 bytes (or 4 in RGBW), so multiply by 3.  An "int" is
 4 bytes, so divide by 4.  The array is created using "int"
 so the compiler will align it to 32 bit memory.
 */

DMAMEM int displayMemory[ledsPerStrip * numPins * 4 / 4];
int drawingMemory[ledsPerStrip * numPins * 4 / 4];

// Initialize Octo library using FastLED Controller
OctoWS2811 octo(ledsPerStrip, displayMemory, drawingMemory,
                WS2811_GRBW | WS2811_800kHz, numPins, pinList);

// Artnet settings
Artnet artnet;

// Check if we got all universes
const int maxUniverses =
    numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

#include "LEDController.h"
LEDController::CTeensy4Controller *pcontroller;
LEDController *ledController;

/**
 * @brief Initialize the LED controller.
 */
void initializeLEDController() {
  octo.begin();
  Debug::println("octo.begin");
  ledController = new LEDController(&octo);
  pcontroller = new LEDController::CTeensy4Controller(&octo, *ledController);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.addLeds(pcontroller, rgbarray, numPins * ledsPerStrip)
      .setCorrection(COLOR_CORRECTION);
  FastLED.delay(10000 / FRAMES_PER_SECOND);
  Debug::println("init test");
  Debug::println("________________INIT TEST_________________");
  FastLED.setBrightness(20);
  ledController = new LEDController(&octo);
  ledController->initTest();
  Debug::println("________________END INIT TEST_________________");
  FastLED.setBrightness(BRIGHTNESS);
}

/**
 * @brief Initialize the Artnet settings.
 */
void initializeArtnet() {
  if (artnet_set == 1) {
    uint8_t mac[6];
    TeensyID::getMAC(mac);
    Ethernet.setSubnetMask({255, 255, 0, 0});
    Debug::printf("IP = %d.%d.%d.%0d\n", ip[0], ip[1], ip[2], ip[3]);
    artnet.begin(mac, ip);
    Debug::println("artnet.begin");
  } else {
    Debug::println("Artnet not set");
  }

  artnet.setArtDmxCallback([](uint16_t universe, uint16_t length,
                              uint8_t sequence, uint8_t *data,
                              IPAddress remoteIP) {
    ledController->onDmxFrameFull(universe, length, sequence, data);
  });

  Debug::println("artnet.setArtDmxCallback");
}

/**
 * @brief Setup function for initializing the system.
 */
void setup() {
  if (debug_set)
    Debug::enable();
  Debug::println("KXKM Etendard");
  Debug::println("Teensy OctoWS28 Artnet Node");
  Debug::println("=================");

  TeensyID::initializeEEPROM(ip);
  initializeLEDController();
  initializeArtnet();

  ledController->setStatusPin(
      blackout_pin); // Replace PIN with the actual pin number
  ledController->setStripType(
      true); // Set to true if using RGBW strip, false if using RGB strip
}

/**
 * @brief Main loop function.
 */
void loop() {
  // we call the read function inside the loop
  if (artnet_set == 1) {
    artnet.read();
  } else {
    delay(1000);
    ledController->initTest();
  }
}
