/**
 * @file Debug.h
 * @brief Fichier d'en-tête pour la classe Debug.
 * @details Une classe pour gérer les messages de débogage.
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

#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

/**
 * @class Debug
 * @brief Classe pour gérer les messages de débogage.
 */
class Debug {
public:
  static bool DEBUG;

  /**
   * @brief Activer le débogage.
   */
  static void enable() {
    DEBUG = true;
    Serial.begin(115200);
  }

  /**
   * @brief Désactiver le débogage.
   */
  static void disable() {
    DEBUG = false;
    Serial.end();
  }

  /**
   * @brief Imprimer un message.
   * @param message Le message à imprimer.
   */
  static void print(const char* message) {
    if (DEBUG) {
      Serial.print(message);
    }
  }

  /**
   * @brief Imprimer une valeur entière.
   * @param value La valeur à imprimer.
   */
  static void print(uint16_t value) {
    if (DEBUG) {
      Serial.print(value);
    }
  }

  /**
   * @brief Imprimer un message avec un saut de ligne.
   * @param message Le message à imprimer.
   */
  static void println(const char* message) {
    if (DEBUG) {
      Serial.println(message);
    }
  }

  /**
   * @brief Imprimer une valeur booléenne avec un saut de ligne.
   * @param value La valeur à imprimer.
   */
  static void println(bool value) {
    if (DEBUG) {
      Serial.println(value);
    }
  }

  /**
   * @brief Imprimer une valeur entière avec un saut de ligne.
   * @param value La valeur à imprimer.
   */
  static void println(uint16_t value) {
    if (DEBUG) {
      Serial.println(value);
    }
  }

  /**
   * @brief Imprimer un message formaté.
   * @param format Le format du message.
   * @param ... Les arguments du message.
   */
  static void printf(const char* format, ...) {
    if (DEBUG) {
      va_list args;
      va_start(args, format);
      Serial.printf(format, args);
      va_end(args);
    }
  }
};

bool Debug::DEBUG = true;

#endif // DEBUG_H
