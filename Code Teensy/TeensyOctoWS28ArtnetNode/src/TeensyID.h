/**
 * @file TeensyID.h
 * @brief Fichier d'en-tête pour la récupération de l'adresse MAC du Teensy.
 * @details Ce fichier contient les définitions nécessaires pour récupérer l'adresse MAC du Teensy.
 * @version V0.2.1
 * @date 2023
 * 
 * @copyright GNU General Public License v3.0
 * 
 * Ce programme est un logiciel libre : vous pouvez le redistribuer et/ou le modifier
 * selon les termes de la Licence Publique Générale GNU publiée par la Free Software Foundation,
 * soit la version 3 de la licence, soit (à votre choix) toute version ultérieure.
 * 
 * Ce programme est distribué dans l'espoir qu'il sera utile,
 * mais SANS AUCUNE GARANTIE ; sans même la garantie implicite de
 * QUALITÉ MARCHANDE ou d'ADÉQUATION À UN USAGE PARTICULIER. Voir la
 * Licence Publique Générale GNU pour plus de détails.
 * 
 * Vous devriez avoir reçu une copie de la Licence Publique Générale GNU
 * avec ce programme. Si ce n'est pas le cas, voir <https://www.gnu.org/licenses/>.
 */

#ifndef TEENSYID_H
#define TEENSYID_H

#include <Arduino.h>
#include <EEPROM.h>
#include "Debug.h"

#if defined(HW_OCOTP_MAC1) && defined(HW_OCOTP_MAC0)
#define HAS_KINETIS_FLASH_FTFA
#elif defined(HAS_KINETIS_FLASH_FTFL)
#define HAS_KINETIS_FLASH_FTFL
#elif defined(HAS_KINETIS_FLASH_FTFE)
#define HAS_KINETIS_FLASH_FTFE
#endif

/**
 * @class TeensyID
 * @brief Classe pour gérer l'ID et l'adresse MAC du Teensy.
 */
class TeensyID {
public:
  /**
   * @brief Récupérer l'adresse MAC du Teensy.
   * @param mac Pointeur vers un tableau d'octets pour stocker l'adresse MAC.
   */
  static void getMAC(uint8_t *mac);

  /**
   * @brief Initialiser les paramètres de l'EEPROM.
   * @param ip Pointeur vers un tableau d'octets pour stocker l'adresse IP.
   */
  static void initializeEEPROM(byte *ip);
};

void TeensyID::getMAC(uint8_t *mac) {
  static char teensyMac[23];
#if defined(HW_OCOTP_MAC1) && defined(HW_OCOTP_MAC0)
  Debug::println("using HW_OCOTP_MAC* - see https://forum.pjrc.com/threads/57595-Serial-amp-MAC-Address-Teensy-4-0");
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
  Debug::println("using FTFL_FSTAT_FTFA - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  FTFL_FCCOB0 = 0x41;
  FTFL_FCCOB1 = 15;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF))
    ; // wait
  SN = *(uint32_t *)&FTFL_FCCOB7;
#define MAC_OK
#elif defined(HAS_KINETIS_FLASH_FTFE)
  Debug::println("using FTFL_FSTAT_FTFE - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
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
  Debug::println(teensyMac);
#else
  Debug::println("ERROR: could not get MAC");
#endif
}

void TeensyID::initializeEEPROM(byte *ip) {
  #ifdef ID_ETENDARD
  if (EEPROM.read(10) != ID_ETENDARD) {
    EEPROM.write(10, ID_ETENDARD);
    delay(200);
  }
  #endif

  if (EEPROM.read(10) == 0 || EEPROM.read(10) >= 255)
  {
    Debug::println("EEPROM not set.. using defaut id: 254");
    EEPROM.write(10, 254); 
  }
  delay(500);
  Debug::printf("EEPROM.read(10) = %d \n", EEPROM.read(10));
  ip[3] = EEPROM.read(10); // Correction de l'affectation de l'adresse IP
}

#endif // TEENSYID_H
