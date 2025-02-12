/**
 * @file LEDController.h
 * @brief Fichier d'en-tête pour la classe LEDController.
 * @details Une classe pour contrôler les bandes LED en utilisant OctoWS2811 et
 * FastLED.
 * @version V0.2.1
 * @date 2023
 *
 * @copyright GNU General Public License v3.0
 *
 * Ce programme est un logiciel libre : vous pouvez le redistribuer et/ou le
 * modifier selon les termes de la Licence Publique Générale GNU publiée par la
 * Free Software Foundation, soit la version 3 de la licence, soit (à votre
 * choix) toute version ultérieure.
 *
 * Ce programme est distribué dans l'espoir qu'il sera utile,
 * mais SANS AUCUNE GARANTIE ; sans même la garantie implicite de
 * QUALITÉ MARCHANDE ou d'ADÉQUATION À UN USAGE PARTICULIER. Voir la
 * Licence Publique Générale GNU pour plus de détails.
 *
 * Vous devriez avoir reçu une copie de la Licence Publique Générale GNU
 * avec ce programme. Si ce n'est pas le cas, voir
 * <https://www.gnu.org/licenses/>.
 */

#include "Debug.h"
#include <Artnet.h>

/**
 * @class LEDController
 * @brief Une classe pour contrôler les bandes LED en utilisant OctoWS2811 et
 * FastLED.
 */
class LEDController : public CPixelLEDController<RGB, 8, 0xFF> {
  OctoWS2811 *pocto;
  unsigned long lastFrameTime;
  int statusPin;
  bool isRGBW;
  int flip = 0;
  int numLeds;
  CRGB *rgbarray;
  bool sendFrame;
  int startUniverse;
  int maxUniverses;
  bool *universesReceived;
  int previousDataLength;

public:
  /**
   * @brief Constructeur pour LEDController.
   * @param _pocto Pointeur vers l'objet OctoWS2811.
   */
  LEDController(OctoWS2811 *_pocto);

  /**
   * @brief Initialiser le motif de test LED.
   */
  void initTest();

  /**
   * @brief Définir les valeurs RGB pour toutes les LEDs.
   * @param r Valeur rouge.
   * @param g Valeur verte.
   * @param b Valeur bleue.
   */
  void setRGB(int r, int g, int b);

  /**
   * @brief Vérifier si une trame Artnet/DMX a été reçue et mettre à jour l'état
   * de la pin.
   * @return True si une trame a été reçue dans les 60 dernières secondes, false
   * sinon.
   */
  bool hasReceivedFrame();

  /**
   * @brief Définir la pin pour indiquer l'état de hasReceivedFrame.
   * @param pin Le numéro de la pin à définir.
   */
  void setStatusPin(int pin);

  /**
   * @brief Définir le type de bande LED (RGB ou RGBW).
   * @param rgbw True si la bande est RGBW, false si elle est RGB.
   */
  void setStripType(bool rgbw);

  /**
   * @brief Convertir les valeurs RGB en valeur blanche pour les bandes RGBW.
   * @param r Valeur rouge.
   * @param g Valeur verte.
   * @param b Valeur bleue.
   * @return Valeur blanche.
   */
  uint8_t whiteFromRGB(uint8_t &r, uint8_t &g, uint8_t &b);

  virtual void init() {}
  virtual void showPixels(PixelController<RGB, 8, 0xFF> &pixels);

  /**
   * @brief Gérer les trames Artnet/DMX entrantes.
   * @param universe Numéro de l'univers.
   * @param length Longueur des données.
   * @param sequence Numéro de séquence.
   * @param data Pointeur vers le tableau de données.
   */
  void onDmxFrameFull(uint16_t universe, uint16_t length, uint8_t sequence,
                      uint8_t *data);

  /**
   * @brief Retourner l'état marche/arrêt basé sur la valeur de flip.
   * @return True si (flip / 15) % 2 == 0, false sinon.
   */
  bool onoff();

  /**
   * @class CTeensy4Controller
   * @brief Une classe pour contrôler les bandes LED en utilisant OctoWS2811 et
   * FastLED.
   */
  class CTeensy4Controller : public CPixelLEDController<RGB, 8, 0xFF> {
    OctoWS2811 *pocto;
    LEDController &ledController;

  public:
    /**
     * @brief Constructeur pour CTeensy4Controller.
     * @param _pocto Pointeur vers l'objet OctoWS2811.
     * @param _ledController Référence à l'objet LEDController.
     */
    CTeensy4Controller(OctoWS2811 *_pocto, LEDController &_ledController)
        : pocto(_pocto), ledController(_ledController) {}

    virtual void init() {}
    virtual void showPixels(PixelController<RGB, 8, 0xFF> &pixels) {
      uint32_t i = 0;
      while (pixels.has(1)) {
        uint8_t r = pixels.loadAndScale0();
        uint8_t g = pixels.loadAndScale1();
        uint8_t b = pixels.loadAndScale2();
        uint8_t w = ledController.whiteFromRGB(r, g, b);
        pocto->setPixel(i++, r, g, b, w);

        pixels.stepDithering();
        pixels.advanceData();
      }
      pocto->show();
    }
  };
};

LEDController::LEDController(OctoWS2811 *_pocto)
    : pocto(_pocto), lastFrameTime(0), statusPin(-1), isRGBW(false), numLeds(0),
      rgbarray(nullptr), sendFrame(false), startUniverse(0), maxUniverses(0),
      universesReceived(nullptr), previousDataLength(0) {}

void LEDController::initTest() {
  const int delaytime = 200;

  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Red;
  FastLED.show();
  delay(delaytime);

  Debug::println("\t DRAW LED RED");

  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Green;
  FastLED.show();
  delay(delaytime);

  Debug::println("\t DRAW LED GREEN");

  delay(delaytime);
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Blue;
  FastLED.show();
  delay(delaytime);

  Debug::println("\t DRAW LED BLUE");

  delay(delaytime);
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::White;
  FastLED.show();
  delay(delaytime);

  Debug::println("\t DRAW LED WHITE");

  delay(delaytime);
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB::Black;
  FastLED.show();
  delay(delaytime);

  Debug::println("\t DRAW LED BLACK");
}

void LEDController::setRGB(int r, int g, int b) {
  for (int i = 0; i < numLeds; i++)
    rgbarray[i] = CRGB(r, g, b);
  FastLED.show();
}

bool LEDController::hasReceivedFrame() {
  unsigned long currentTime = millis();
  if (statusPin != -1) {
    if (currentTime - lastFrameTime < 60000) {
      digitalWrite(statusPin, HIGH);
      return true;
    } else if (currentTime - lastFrameTime > 300000) {
      digitalWrite(statusPin, LOW);
    }
  }
  return false;
}

void LEDController::setStatusPin(int pin) {
  statusPin = pin;
  pinMode(statusPin, OUTPUT);
}

void LEDController::setStripType(bool rgbw) { isRGBW = rgbw; }

uint8_t LEDController::whiteFromRGB(uint8_t &r, uint8_t &g, uint8_t &b) {
  if (isRGBW) {
    uint8_t w = min(r, min(g, b));
    r -= w;
    g -= w;
    b -= w;
    return w;
  }
  return 0;
}

void LEDController::showPixels(PixelController<RGB, 8, 0xFF> &pixels) {
  uint32_t i = 0;
  while (pixels.has(1)) {
    uint8_t r = pixels.loadAndScale0();
    uint8_t g = pixels.loadAndScale1();
    uint8_t b = pixels.loadAndScale2();
    uint8_t w = whiteFromRGB(r, g, b);
    pocto->setPixel(i++, r, g, b, w);

    pixels.stepDithering();
    pixels.advanceData();
  }
  pocto->show();
}

void LEDController::onDmxFrameFull(uint16_t universe, uint16_t length,
                                   uint8_t sequence, uint8_t *data) {
  sendFrame = 1;
  lastFrameTime = millis();

  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0; i < maxUniverses; i++) {
    if (universesReceived[i] == 0) {
      sendFrame = 0;
      break;
    }
  }

  if (Debug::DEBUG) {
    Debug::printf("universe number = %d\tdata length = %d\tDMX data[0]: "
                  "%d\tsendFrame = %d\n",
                  artnet.getUniverse(), artnet.getLength(), data[0], sendFrame);
  }

  for (int i = 0; i < length / 3; i++) {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < numLeds) {
      rgbarray[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }

  previousDataLength = length;

  if (sendFrame) {
    if (Debug::DEBUG)
      Debug::println("\t DRAW LEDs");
    FastLED.show();
    flip += 1;

    memset(universesReceived, 0, maxUniverses);
    previousDataLength = 0;
  }
}

bool LEDController::onoff() { return (flip / 15) % 2 == 0; }
