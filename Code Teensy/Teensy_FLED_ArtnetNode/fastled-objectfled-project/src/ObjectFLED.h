#ifndef OBJECTFLED_H
#define OBJECTFLED_H

#include <FastLED.h>

class ObjectFLED {
public:
    ObjectFLED(uint16_t numLeds, CRGB* leds, EOrder order, uint8_t numStrings, uint8_t* pinList);
    void begin(float overclockFactor);
    void show();
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const;
    void setBalance(uint8_t balance);
    uint8_t getBalance() const;

private:
    CRGB* _leds;
    uint16_t _numLeds;
    uint8_t _brightness;
    uint8_t _balance;
    EOrder _order;
    uint8_t _numStrings;
    uint8_t* _pinList;
};

#endif // OBJECTFLED_H