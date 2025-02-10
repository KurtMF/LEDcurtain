#include <ObjectFLED.h>
#include <FastLED.h>

#define PIX_PER_STR 32
#define NUM_STR 5
CRGB leds[NUM_STR][PIX_PER_STR];      
uint8_t pinList[NUM_STR] = {0, 1, 17, 3, 4};
ObjectFLED dispLeds(PIX_PER_STR * NUM_STR, leds, CORDER_RGB, 5, pinList);

void setup() {
  dispLeds.begin(1.5);                
  fill_solid(leds[0], NUM_STR * PIX_PER_STR, 0x0);    
  dispLeds.setBrightness(10);
  Serial.begin(100000);
}

int delayT = 30;
void loop() {
  for (uint x = 0; x < PIX_PER_STR; x++) {
    for (uint y = 0; y < NUM_STR; y++) {
      leds[y][x] = CRGB::Blue;
    }
    dispLeds.show();
    fadeToBlackBy(leds[0], NUM_STR * PIX_PER_STR, 32);
    delay(delayT);
  }

  for (int x = PIX_PER_STR - 1; x >= 0; x--) {
    for (int y = NUM_STR - 1; y >= 0; y--) {
      leds[y][x] = CRGB::Blue;
    }
    dispLeds.show();
    fadeToBlackBy(leds[0], NUM_STR * PIX_PER_STR, 32);
    delay(delayT);
  }

  uint totalT, startT = micros();
  for (int i = 0; i < 20; i++) {
    dispLeds.show();
  }
  totalT = micros() - startT;
  Serial.printf("Uses ObjectFLED Avg T of 20 :  %.1f uS  FPS:  %.1f\n", totalT / 20.0, 20.0 / totalT * 1000000);
  while (Serial.read() == -1);
}