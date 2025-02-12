# Teensy OctoWS28 Artnet Node

## Description

Ce projet implémente un nœud Artnet utilisant un Teensy 4.1 et OctoWS2811 pour contrôler des bandes LED. Il permet de recevoir des données DMX via Artnet et de les afficher sur des bandes LED connectées au Teensy.

## Matériel requis

- [Teensy 4.1](https://www.pjrc.com/store/teensy41.html)
- [OctoWS2811](https://www.pjrc.com/teensy/td_libs_OctoWS2811.html)
- Bandes LED compatibles WS2811
- [Ethernet Shield pour Teensy](https://www.pjrc.com/store/ethernet_kit.html)

## Installation

1. Clonez ce dépôt sur votre machine locale :
    ```sh
    git clone https://github.com/votre-utilisateur/TeensyOctoWS28ArtnetNode.git
    ```

2. Ouvrez le projet avec PlatformIO.

3. Sélectionnez l'environnement correspondant à votre configuration dans le fichier `platformio.ini`.

4. Compilez et téléversez le code sur votre Teensy 4.1.

## Configuration

Le fichier `platformio.ini` contient les configurations pour différents environnements :

- `etendarvGRAZ` : Configuration pour Etendard V0.1 (GRAZ)
- `etendarv1a` : Configuration pour Etendard V1.a (KXKM)
- `etendarv2a` : Configuration pour Etendard V2.a (KXKM)

Vous pouvez sélectionner l'environnement par défaut en modifiant la ligne suivante dans `platformio.ini` :
```ini
default_envs = etendarv1a
```

## Utilisation

1. Connectez votre Teensy 4.1 à votre réseau via Ethernet.

2. Allumez votre bande LED.

3. Envoyez des données DMX via Artnet à l'adresse IP configurée dans le code.

## Débogage

Le débogage peut être activé ou désactivé en modifiant la constante `debug_set` dans le fichier `main.cpp` :
```cpp
const int debug_set = 1; // 1 pour activer, 0 pour désactiver
```

## Licence

Ce projet est sous licence MIT et GNU General Public License v3.0. Voir le fichier [LICENSE](LICENSE) pour plus de détails.

## Auteurs

- Studio Jordan Shaw
- Clément SAILLANT
- maigre

## Contributeurs

Ce projet utilise plusieurs librairies open-source. Merci aux auteurs et contributeurs de ces librairies :

- [Paul Stoffregen](https://github.com/PaulStoffregen) pour [OctoWS2811](https://github.com/PaulStoffregen/OctoWS2811)
- [Daniel Garcia](https://github.com/focalintent) et [Mark Kriegsman](https://github.com/marmilicious) pour [FastLED](https://github.com/FastLED/FastLED)
- [vjmuzik](https://github.com/vjmuzik) pour [NativeEthernet](https://github.com/vjmuzik/NativeEthernet)

## Compagnie

Ce projet a été développé par la compagnie [Komplex Kapharnaum](https://www.komplex-kapharnaum.net/), une compagnie de création artistique basée à Lyon, France. Komplex Kapharnaum crée des spectacles et des installations artistiques en utilisant des technologies innovantes pour transformer l'espace public et offrir des expériences immersives au public.
