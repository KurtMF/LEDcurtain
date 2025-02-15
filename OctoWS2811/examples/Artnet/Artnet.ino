// Receive multiple universes via Artnet and control a strip of ws2811 leds via OctoWS2811
//
// This example may be copied under the terms of the MIT license, see the LICENSE file for details
//  https://github.com/natcl/Artnet
// 
// http://forum.pjrc.com/threads/24688-Artnet-to-OctoWS2811?p=55589&viewfull=1#post55589

#include <Artnet.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
//#include <OctoWS2811.h>
#include <ObjectFLED.h>

// Ideas for improving performance with WIZ820io / WIZ850io Ethernet:
// https://forum.pjrc.com/threads/45760-E1-31-sACN-Ethernet-DMX-Performance-help-6-Universe-Limit-improvements

// OctoWS2811 settings
const int ledsPerStrip = 492; // change for your setup
const byte numStrips= 1; // change for your setup
byte drawingMemory[numStrips * ledsPerStrip * 3];
const int config = CORDER_GRB;
uint8_t pinList[numStrips] = {10};
ObjectFLED leds(ledsPerStrip * numStrips, drawingMemory, config, numStrips, pinList);

// Artnet settings
Artnet artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as zero.
const int numberOfChannels = ledsPerStrip * numStrips * 3; // Total number of channels you want to receive (1 led = 3 channels)
byte channelBuffer[numberOfChannels]; // Combined universes into a single array

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;

// Change ip and mac address for your setup
byte ip[] = {192, 168, 2, 2};
byte mac[] = {0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC};

void setup()
{
  Serial.begin(115200);
  artnet.begin(mac, ip);
  leds.begin();
  initTest();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;

  // Store which universe has got in
  if (universe < maxUniverses)
    universesReceived[universe] = 1;

  for (int i = 0 ; i < maxUniverses ; i++)
  {
    if (universesReceived[i] == 0)
    {
      //Serial.println("Broke");
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0 ; i < length ; i++)
  {
    int bufferIndex = i + ((universe - startUniverse) * length);
    if (bufferIndex < numberOfChannels) // to verify
      channelBuffer[bufferIndex] = byte(data[i]);
  }      

  // send to leds
  for (int i = 0; i < ledsPerStrip * numStrips; i++)
  {
    memcpy(&drawingMemory[i * 3], &channelBuffer[i * 3], 3);
  } 
  
  if (sendFrame)
  {
    leds.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}

void setPixel(int index, byte r, byte g, byte b) {
  drawingMemory[index * 3] = r;
  drawingMemory[index * 3 + 1] = g;
  drawingMemory[index * 3 + 2] = b;
}  //setPixel()

void initTest()
{
  for (int i = 0 ; i < ledsPerStrip * numStrips ; i++)
    setPixel(i, 127, 0, 0);
  leds.show();
  delay(500);
  for (int i = 0 ; i < ledsPerStrip * numStrips  ; i++)
    setPixel(i, 0, 127, 0);
  leds.show();
  delay(500);
  for (int i = 0 ; i < ledsPerStrip * numStrips  ; i++)
    setPixel(i, 0, 0, 127);
  leds.show();
  delay(500);
  for (int i = 0 ; i < ledsPerStrip * numStrips  ; i++)
    setPixel(i, 0, 0, 0);
  leds.show();
}
