// Teensy OctoWS28 Artnet Node
// Studio Jordan Shaw
// =================
// Version: V0.2.1
// 
// An Artnet node built with Teensy 4.1 + OctoWS28
// Further documentation in the READEME.md 
//


#include <Artnet.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

#include <SPI.h>
#include <OctoWS2811.h>
#include <FastLED.h>


// To help with logevity of LEDs and Octo Board
// Brightness is set to ~50%
#define BRIGHTNESS  10

// Throttling refresh for when using 24+ universes
// ie. 510 leds / 3 universes per pin
#define FRAMES_PER_SECOND  30

// Turn on / off Serial logs for debugging
#define DEBUG 1

// OctoWS2811 settings
const int numPins = 2; // Number of pins used for LED output = 32
// 

// These are the Octo default pins, can be changed as needed
byte pinList[numPins] = { 5, 6};
// 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 13, 14 , 15, 16, 17
// Equivelent to 2 DMX universes
const int ledsPerStrip = 108;

// Octo will always have 8 strips
// Just recasting num of pins as strips for clarity below
// change for your setup
const byte numStrips = numPins;

const int numLeds = ledsPerStrip * numStrips;

// Total number of channels you want to receive (1 led = 3 channels)
const int numberOfChannels = numLeds * 3;

// Define your FastLED pixels
CRGB rgbarray[numPins * ledsPerStrip];

// Memory buffer to artnet data
DMAMEM int displayMemory[ledsPerStrip * numPins * 3 / 4];
int drawingMemory[ledsPerStrip * numPins * 3 / 4];

// Initialize Octo library using FastLED Controller
OctoWS2811 octo(ledsPerStrip, displayMemory, drawingMemory, WS2811_BRGW | WS2811_800kHz, numPins, pinList);  // TODO:  WS2811_400kHz ou WS2811_800kHz ?


template <EOrder RGB_ORDER = RGB,
          uint8_t CHIP = WS2811_800kHz>
class CTeensy4Controller : public CPixelLEDController<RGB_ORDER, 8, 0xFF>
{
    OctoWS2811 *pocto;

    public:
        CTeensy4Controller(OctoWS2811 *_pocto) : pocto(_pocto){};

        virtual void init() {}
        virtual void showPixels(PixelController<RGB_ORDER, 8, 0xFF> &pixels)
        {

            uint32_t i = 0;
            while (pixels.has(1))
            {
                uint8_t r = pixels.loadAndScale0();
                uint8_t g = pixels.loadAndScale1();
                uint8_t b = pixels.loadAndScale2();

                // Build white from RGB
                uint8_t w = min(r, min(g, b));
                r -= w;
                g -= w;
                b -= w;

                pocto->setPixel(i++, g, r, b, w);
                pixels.stepDithering();
                pixels.advanceData();
            }

            pocto->show();
        }
};

// Artnet settings
Artnet artnet;
// CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.
const int startUniverse = 0; 

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

// Change ip and mac address for your setup
// on macos, this mac address is for local host "ether" address
// for me this was the mac address of `en0`
// If using a Pi, you'll want to change it to the Pi's mac address. Remember to update when changing devices or enviriments
byte mac[] = {0x3c, 0x22, 0xfb, 0x87, 0x16, 0x1b};

// Network IP addresses
// On mac using a ethernet dongle, the IP is configured in the OSX network settings.
// You wanna set the IP you set for the dongle as the IP here.
// For every device, the IP will be different. Make sure to update when changing devices or enviroments
byte ip[] = {192, 168, 1, 12};

// From Blinky Light's blog to allow for Fast LED integration + Pin selection
// https://blinkylights.blog/2021/02/03/using-teensy-4-1-with-fastled/
CTeensy4Controller<RGB, WS2811_800kHz> *pcontroller;


#include "function.h"


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
  sendFrame = 1;

  // Store which universe was received
  // =============
  // If your artnet server is not sending the same number of LED data configfured for this node
  // ie: the configured number of LED pixels + universes, the LEDs will not turn on.
  // LED + universe numbers set in this app must match the data the artnet server is sending
  // =============
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0; i < maxUniverses; i++)
  {
    if (DEBUG)
    {
      Serial.print("universesReceived[i] \t");
      Serial.println(i);
      Serial.print("\t");
      Serial.println(universesReceived[i]);
    }

    if (universesReceived[i] == 0)
    {
      sendFrame = 0;
      break;
    }
  }

  if (DEBUG)
  {
    // print out our data
    Serial.print("universe number = ");
    Serial.print(artnet.getUniverse());
    Serial.print("\tdata length = ");
    Serial.print(artnet.getLength());
    Serial.print("\tsequence n0. = ");
    Serial.println(artnet.getSequence());
    Serial.print("DMX data: ");
    // print out MORE data:
    Serial.print("ledsPerStrip = ");
    Serial.print(ledsPerStrip);
    Serial.print("\tmaxUniverses = ");
    Serial.print(maxUniverses);
    Serial.print("\tsendFrame = ");
    Serial.println(sendFrame);
  }

  // Read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < numLeds)
    {
      rgbarray[led] = CRGB(gamma8[data[i * 3]], gamma8[data[i * 3 + 1]], gamma8[data[i * 3 + 2]]);
    }
  }

  previousDataLength = length;

  if (sendFrame)
  {
    if (DEBUG)
    {
      Serial.println("\t DRAW LEDs");
    }

    FastLED.show();

    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}

void setup()
{
  Serial.begin(115200);

  Serial.println("Studio Jordan Shaw");
  Serial.println("Teensy OctoWS28 Artnet Node");
  Serial.println("=================");
  Serial.println("Version: V0.2.1");

  //artnet.begin(mac, ip);
Serial.println("artnet.begin");
  octo.begin();
  Serial.println("octo.begin");
  pcontroller = new CTeensy4Controller<RGB, WS2811_800kHz>(&octo);

  FastLED.addLeds(pcontroller, rgbarray, numPins * ledsPerStrip).setCorrection(TypicalLEDStrip);
  FastLED.delay(1000/FRAMES_PER_SECOND);
  Serial.println("init test");
  initTest();
  Serial.println("________________END INIT TEST_________________");
  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
  Serial.println("artnet.setArtDmxCallback");
}


void loop()
{
  // we call the read function inside the loop
  artnet.read();
  delay(5000);
  //initTest();
}
