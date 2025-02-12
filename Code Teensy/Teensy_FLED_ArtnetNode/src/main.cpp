/*
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

/**
 * @file main.cpp
 * @brief Point d'entrée principal du programme.
 * @details Créé par Clément Saillant @electron-rare pour Komplex Kapharnum
 * Ce programme permet de gérer un système d'écran à base de LED en artnet
 *
 * Pour plus d'informations, consultez le fichier README.md
 * @version 1.0
 * @date 2025-02-11
 */

#include <ObjectFLED.h> // must include this before FastLED.h
// As if FastLED 3.9.12, this is no longer needed for Teensy 4.0/4.1.
#define FASTLED_USES_OBJECTFLED
#include "FastLED.h"
// #include "fl/warn.h" // must include this before FastLED.h

#include <Artnet.h>
#include <EEPROM.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <OctoWS2811.h>
#include <SPI.h>

#include "gamma8.h"

// Turn on / off Serial logs for debugging
#define DEBUG 1
// #define ID_ETENDARD 2
// Set to 1 to enable Artnet
// Set to 0 to disable Artnet and run a test pattern
#define artnet_set 1

// Set time to wait for economy mode
#define ECONOMY_MODE 3000
unsigned long previousMillis = 0;
// To help with longevity of LEDs and Octo Board
// Brightness is set to ~50% (0-255)

// 50% = 30A in full white
#define BRIGHTNESS 150

/*
 COLOR_CORRECTION
   TypicalLEDStrip=    0xFFB0F0     255, 176, 240
   Typical8mmPixel=    0xFFE08C     255, 224, 140
   TypicalPixelString= 0xFFE08C     255, 224, 140
   UncorrectedColor=   0xFFFFFF     255, 255, 255
 */

int flip = 0;
bool onoff() { return (flip / 15) % 2 == 0; }

// #define COLOR_CORRECTION TypicalLEDStrip
#define COLOR_CORRECTION UncorrectedColor

/**
 * @brief Convertit les valeurs RGB en valeur blanche.
 * 
 * @param r Référence à la composante rouge.
 * @param g Référence à la composante verte.
 * @param b Référence à la composante bleue.
 * @return uint8_t Valeur de la composante blanche.
 */
uint8_t white_from_rgb(uint8_t &r, uint8_t &g, uint8_t &b) {
  r = rg8(r);
  g = gg8(g);
  b = bg8(b);

  uint8_t w = min(r, min(g, b));
  r -= w;
  g -= w;
  b -= w;

  return w;
}

// Throttling refresh for when using 24+ universes
// i.e. 510 leds / 3 universes per pin
#define FRAMES_PER_SECOND 30

// CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet
// first universe as 0.
const int startUniverse = 0;

// Network IP addresses
// On mac using an ethernet dongle, the IP is configured in the OSX network
// settings. You wanna set the IP you set for the dongle as the IP here. For
// every device, the IP will be different. Make sure to update when changing
// devices or environments

byte ip[] = {2, 12, 0, 254}; // IP address of the node (254 is default, will be
                             // overwritten by ID_ETENDARD read from EEPROM)

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
const byte pinList[numPins] = {1,  2,  3,  4,  19, 20, 21, 22, 5,  6,
                               7,  8,  9,  10, 11, 12, 24, 25, 26, 27,
                               28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
                               38, 39, 40, 41, 13, 14}; // List of pins used for
                                                        // LED output
const int Led_for_one_strip = 108; // Number of LEDs per strip
const int Nb_string_strip = 1;     // Number of strips per pin
#endif

const int ledsPerStrip = Led_for_one_strip * Nb_string_strip;
// Octo will always have 8 strips
// Just recasting num of pins as strips for clarity below
// change for your setup
const byte numStrips = numPins;

const int numLeds = ledsPerStrip * numStrips;
const int PIX_PER_STR = ledsPerStrip;
// Total number of channels you want to receive (1 led = 3 channels)
const int numberOfChannels = numLeds * 3;

// Define your FastLED pixels
CRGB rgbarray[numPins * ledsPerStrip];
ObjectFLED dispLeds(PIX_PER_STR *numStrips, rgbarray, CORDER_GRBW, numStrips,
                    pinList);

/**
 * @brief Contrôleur pour Teensy 4 utilisant ObjectFLED.
 * 
 * @tparam RGB_ORDER Ordre des couleurs RGB.
 * @tparam CHIP Type de puce LED.
 */
template <EOrder RGB_ORDER = RGB, uint8_t CHIP = WS2811_800kHz>
class CTeensy4Controller : public CPixelLEDController<RGB_ORDER, 8, 0xFF> {
  ObjectFLED *pobjectFled;

public:
  /**
   * @brief Constructeur du contrôleur.
   * 
   * @param _pobjectFled Pointeur vers l'objet ObjectFLED.
   */
  CTeensy4Controller(ObjectFLED *_pobjectFled) : pobjectFled(_pobjectFled){};

  /**
   * @brief Initialisation du contrôleur.
   */
  virtual void init() {}

  /**
   * @brief Affiche les pixels.
   * 
   * @param pixels Contrôleur de pixels.
   */
  virtual void showPixels(PixelController<RGB_ORDER, 8, 0xFF> &pixels) {

    uint32_t i = 0;
    while (pixels.has(1)) {
      uint8_t r = pixels.loadAndScale0();
      uint8_t g = pixels.loadAndScale1();
      uint8_t b = pixels.loadAndScale2();
      uint8_t w = white_from_rgb(r, g, b);
      pobjectFled->setPixel(i++, g, r, b, w);

      pixels.stepDithering();
      pixels.advanceData();
    }

    pobjectFled->show();
  }
};

CTeensy4Controller<RGB, WS2811_800kHz> *pcontroller;

// Artnet settings
Artnet artnet;

// Check if we got all universes
const int maxUniverses =
    numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

#include "function.h"

/**
 * @brief Callback pour traiter les trames DMX complètes.
 * 
 * @param universe Numéro de l'univers.
 * @param length Longueur des données.
 * @param sequence Séquence des données.
 * @param data Pointeur vers les données.
 */
void onDmxFrame_full(uint16_t universe, uint16_t length, uint8_t sequence,
                     uint8_t *data) {
  sendFrame = 1;

  // Store which universe was received
  // =============
  // If your artnet server is not sending the same number of LED data
  // configured for this node i.e.: the configured number of LED pixels +
  // universes, the LEDs will not turn on. LED + universe numbers set in this
  // app must match the data the artnet server is sending
  // =============
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0; i < maxUniverses; i++) {
    /*
    if (DEBUG) {
      Serial.print("universesReceived[i] \t");
      Serial.println(i);
      Serial.print("\t");
      Serial.println(universesReceived[i]);
    }
    */
    if (universesReceived[i] == 0) {
      sendFrame = 0;
      break;
    }
  }

  // Read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++) {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < numLeds) {
      rgbarray[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }

  previousDataLength = length;
  // set the LED on if the data is not 0
  if (data[0] != 0) {
    digitalWrite(23, HIGH);
    previousMillis = millis();
  }
  if (sendFrame) {
    if (DEBUG)
      Serial.println("\t DRAW LEDs");
    dispLeds.show();
    flip += 1;

    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
    previousDataLength = 0;
  }
  // else if (DEBUG) Serial.println("\t NOT DRAW LEDs ");
}

/**
 * @brief Fonction d'initialisation du programme.
 */
void setup() {

// Rewrite nodeID
#ifdef ID_ETENDARD
  if (EEPROM.read(10) != ID_ETENDARD) {
    EEPROM.write(10, ID_ETENDARD);
    delay(200);
  }
#endif

  if (EEPROM.read(10) == 0 || EEPROM.read(10) >= 255) {
    if (DEBUG)
      Serial.println("EEPROM not set.. using default id: 254");
    EEPROM.write(10, 254);
  }
  delay(500);
  if (DEBUG)
    Serial.printf("EEPROM.read(10) = %d \n", EEPROM.read(10));
  ip[3] = {EEPROM.read(10)}; // IP address of the node
  if (DEBUG) {
    Serial.begin(115200);
    Serial.println("KXKM Etendard");
    Serial.println("Teensy OctoWS28 Artnet Node");
    Serial.println("=================");
  }
  // begin(LED_CLK_nS, LED_T0H_nS, LED_T1H_nS, LED_Latch_Delay_uS) - specifies
  // full LED waveform timing.
  // TODO set the timing for the LED strip
  dispLeds.begin(1250, 300, 600,
                 1000); // Utiliser ObjectFLED au lieu de OctoWS2811

  if (DEBUG)
    Serial.println("dispLeds.begin");
  pcontroller = new CTeensy4Controller<RGB, WS2811_800kHz>(&dispLeds);
  if (DEBUG)
  Serial.println("pcontroller");
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.addLeds(pcontroller, rgbarray, numPins * ledsPerStrip)
      .setCorrection(COLOR_CORRECTION);
      if (DEBUG)
  Serial.println("add led");
  FastLED.delay(10000 / FRAMES_PER_SECOND);
  if (DEBUG)
  Serial.println("fps");
  dispLeds.setBrightness(BRIGHTNESS);
  if (DEBUG)
    Serial.println("init test");
  if (DEBUG)
    Serial.println("________________INIT TEST_________________");
  dispLeds.setBrightness(20);
  initTest();
  if (DEBUG)
    Serial.println("________________END INIT TEST_________________");
  dispLeds.setBrightness(BRIGHTNESS);

  if (artnet_set == 1) {
    uint8_t mac[6];
    teensyMAC(mac);
    Ethernet.setSubnetMask({255, 255, 0, 0});
    if (DEBUG)
      Serial.printf("IP = %d.%d.%d.%0d\n", ip[0], ip[1], ip[2], ip[3]);
    artnet.begin(mac, ip);
    if (DEBUG)
      Serial.println("artnet.begin");
  } else if (DEBUG)
    Serial.println("Artnet not set");

  // this will be called for each packet received

  artnet.setArtDmxCallback(onDmxFrame_full);

  if (DEBUG)
    Serial.println("artnet.setArtDmxCallback");
}

/**
 * @brief Définit la couleur RGB pour toutes les LEDs.
 * 
 * @param r Valeur de la composante rouge.
 * @param g Valeur de la composante verte.
 * @param b Valeur de la composante bleue.
 */
void set_rgb(int r, int g, int b) {
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB(r, g, b);
  FastLED.show();
}

/**
 * @brief Boucle principale du programme.
 */
void loop() {
  // we call the read function inside the loop
  if (artnet_set == 1) {
    artnet.read();
  } else {
    delay(1000);
    initTest();
  }
  if (millis() - previousMillis > ECONOMY_MODE)
    digitalWrite(23, LOW);
}
