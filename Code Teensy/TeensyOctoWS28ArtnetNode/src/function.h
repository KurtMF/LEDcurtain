
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

void teensyMAC(uint8_t *mac) // recover mac address from teensy
{
  static char teensyMac[23];
#if defined(HW_OCOTP_MAC1) && defined(HW_OCOTP_MAC0)
  Serial.println("using HW_OCOTP_MAC* - see https://forum.pjrc.com/threads/57595-Serial-amp-MAC-Address-Teensy-4-0");
  for (uint8_t by = 0; by < 2; by++)
    mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
  for (uint8_t by = 0; by < 4; by++)
    mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
#define MAC_OK
#else
  mac[0] = 0x04;
  mac[1] = 0xE9;
  mac[2] = 0xE5;
  uint32_t SN = 0;
  __disable_irq();
#if defined(HAS_KINETIS_FLASH_FTFA) || defined(HAS_KINETIS_FLASH_FTFL)
  Serial.println("using FTFL_FSTAT_FTFA - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  FTFL_FCCOB0 = 0x41;
  FTFL_FCCOB1 = 15;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF))
    ; // wait
  SN = *(uint32_t *)&FTFL_FCCOB7;
#define MAC_OK
#elif defined(HAS_KINETIS_FLASH_FTFE)
  Serial.println("using FTFL_FSTAT_FTFE - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
  kinetis_hsrun_disable();
  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  *(uint32_t *)&FTFL_FCCOB3 = 0x41070000;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF))
    ; // wait
  SN = *(uint32_t *)&FTFL_FCCOBB;
  kinetis_hsrun_enable();
#define MAC_OK
#endif
  __enable_irq();
  for (uint8_t by = 0; by < 3; by++)
    mac[by + 3] = (SN >> ((2 - by) * 8)) & 0xFF;
#endif

#ifdef MAC_OK
  sprintf(teensyMac, "MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(teensyMac);
  Serial.printf("IP = %d.%d.%d.%0d\n", ip[0], ip[1], ip[2], ip[3]);
#else
  Serial.println("ERROR: could not get MAC");
#endif
}
