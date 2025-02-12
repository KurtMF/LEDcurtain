// Blink some lights from setup(), to confirm upload
void initTest() {
  const int delaytime = 200;

  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Red;
  dispLeds.show();
  delay(delaytime);
  if (DEBUG) {
    Serial.println("\t DRAW LED RED");
  }
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Green;
  dispLeds.show();
  delay(delaytime);

  if (DEBUG) {
    Serial.println("\t DRAW LED GREEN");
  }
  delay(delaytime);
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Blue;
  dispLeds.show();
  delay(delaytime);

  if (DEBUG) {
    Serial.println("\t DRAW LED BLUE");
  }
  delay(delaytime);
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::White;
  dispLeds.show();
  delay(delaytime);

  if (DEBUG) {
    Serial.println("\t DRAW LED WHITE");
  }
  delay(delaytime);
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Black;
  dispLeds.show();
  delay(delaytime);

  if (DEBUG) {
    Serial.println("\t DRAW LED BLACK");
  }
}

void teensyMAC(uint8_t *mac) // recover mac address from teensy
{
  static char teensyMac[23];
#if defined(HW_OCOTP_MAC1) && defined(HW_OCOTP_MAC0)
  Serial.println(
      "using HW_OCOTP_MAC* - see "
      "https://forum.pjrc.com/threads/57595-Serial-amp-MAC-Address-Teensy-4-0");
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
  Serial.println("using FTFL_FSTAT_FTFA - vis teensyID.h - see "
                 "https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  FTFL_FCCOB0 = 0x41;
  FTFL_FCCOB1 = 15;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF))
    ; // wait
  SN = *(uint32_t *)&FTFL_FCCOB7;
#define MAC_OK
#elif defined(HAS_KINETIS_FLASH_FTFE)
  Serial.println("using FTFL_FSTAT_FTFE - vis teensyID.h - see "
                 "https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
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
  sprintf(teensyMac, "MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],
          mac[2], mac[3], mac[4], mac[5]);
  Serial.println(teensyMac);
#else
  Serial.println("ERROR: could not get MAC");
#endif
}

void eeprom_set_IP(int ID_ETENDARD) {
  // Rewrite nodeID
  if (EEPROM.read(10) != ID_ETENDARD) {
    EEPROM.write(10, ID_ETENDARD);
    delay(200);
  }

  if (EEPROM.read(10) == 0 || EEPROM.read(10) >= 255) {
    if (DEBUG)
      Serial.println("EEPROM not set.. using default id: 2");
    EEPROM.write(10, 2);
  }
  delay(500);
  if (DEBUG)
    Serial.printf("EEPROM.read(10) = %d \n", EEPROM.read(10));
  ip[3] = {EEPROM.read(10)}; // IP address of the node
  if (DEBUG)
    Serial.printf("EEPROM IP = %d.%d.%d.%0d\n", ip[0], ip[1], ip[2], ip[3]);
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
 * @brief Définit la balance des couleurs RGB.
 *
 * @param r Valeur de la composante rouge de 0 à 255.
 * @param g Valeur de la composante verte de 0 à 255.
 * @param b Valeur de la composante bleue de 0 à 255.
 */
void setColorBalance(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t balance = (r << 16) | (g << 8) | b;
  FastLED.setCorrection(balance);
}

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

// not used in this version because we use ObjectFLED and setpentin is managed by the library
int flip = 0;
bool onoff() { return (flip / 15) % 2 == 0; }