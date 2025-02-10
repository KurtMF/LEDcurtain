# fastled-objectfled-project/fastled-objectfled-project/README.md

# Projet FastLED avec ObjectFLED

Ce projet utilise la bibliothèque FastLED en conjonction avec ObjectFLED pour contrôler plusieurs chaînes de LED. Il permet d'afficher des motifs lumineux et de mesurer les performances de l'affichage.

## Fichiers du projet

- `src/main.cpp` : Contient le code principal de l'application. Il initialise les LED avec FastLED et configure ObjectFLED pour gérer plusieurs chaînes de LED. Des fonctions sont incluses pour afficher des motifs lumineux et mesurer les performances.

- `src/ObjectFLED.h` : En-tête de la bibliothèque ObjectFLED. Définit la classe ObjectFLED, qui gère l'affichage des LED, y compris les méthodes pour initialiser les LED, définir la luminosité et afficher les motifs.

- `platformio.ini` : Configuration pour PlatformIO. Spécifie les dépendances du projet, les environnements de construction et les paramètres de compilation nécessaires.

## Instructions de configuration

1. Clonez le dépôt ou téléchargez les fichiers du projet.
2. Ouvrez le projet dans votre environnement de développement.
3. Assurez-vous que les bibliothèques FastLED et ObjectFLED sont installées.
4. Compilez le projet en utilisant PlatformIO.

## Exécution

Après la compilation, téléversez le code sur votre microcontrôleur. Les LED devraient s'initialiser et commencer à afficher les motifs lumineux définis dans `main.cpp`.

## Auteurs

Ce projet a été développé par [Votre Nom].