
// Blink some lights from setup(), to confirm upload
void initTest()
{
  const int delaytime = 200;

  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Red;
  FastLED.show();
  delay(delaytime);
  if (DEBUG)
  {
    Serial.println("\t DRAW LED RED");
  }
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Green;
  FastLED.show();
  delay(delaytime);

  if (DEBUG)
  {
    Serial.println("\t DRAW LED GREEN");
  }
  delay(delaytime);
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Blue;
  FastLED.show();
  delay(delaytime);

  if (DEBUG)
  {
    Serial.println("\t DRAW LED BLUE");
  }
  delay(delaytime);
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::White;
  FastLED.show();
  delay(delaytime);

  if (DEBUG)
  {
    Serial.println("\t DRAW LED WHITE");
  }
  delay(delaytime);
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Black;
  FastLED.show();
  delay(delaytime);

  if (DEBUG)
  {
    Serial.println("\t DRAW LED BLACK");
  }
}