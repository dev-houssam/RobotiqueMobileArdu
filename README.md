# RobotiqueMobileArdu

# Projet robotique — Robot mobile Arduino

Projet collectif de robotique réalisé dans le cadre d'un TP en octobre 2023.
L'objectif était de programmer un robot mobile basé sur **Arduino UNO**, capable de recevoir des séquences de commandes depuis un ordinateur, de piloter ses moteurs et d'exploiter des capteurs de distance.

## Présentation

Le robot est constitué d'une carte **Arduino UNO**, de deux moteurs commandés en PWM et de deux capteurs de distance **VL53L1X**.

L'Arduino reçoit les commandes sous forme de texte via une communication série. Les commandes sont regroupées dans une séquence, analysées puis exécutées successivement.

Le programme assure également :

* le pilotage indépendant des moteurs gauche et droit ;
* la gestion de la marche avant et arrière ;
* l'arrêt des moteurs ;
* la limitation de la durée d'exécution d'une séquence ;
* la surveillance des distances ;
* l'arrêt automatique en fonction d'une distance de sécurité ;
* le retour des mesures des capteurs au format JSON.

## Architecture

```text
                    USB / XBee
Ordinateur  <-------------------------->  Arduino UNO
                                             |
                              +--------------+--------------+
                              |                             |
                         Commande moteurs              Capteurs
                              |                             |
                       +------+------+                 +----+----+
                       |             |                 |         |
                  Moteur gauche  Moteur droit      VL53L1X   VL53L1X
```

## Protocole de commandes

Une séquence de commandes est transmise sous la forme :

```text
[[commande paramètre][commande paramètre]...]
```

Exemple :

```text
[[ns 3][mga 255][mda 255][t 1000]]
```

Cette séquence permet notamment de :

1. définir le numéro de séquence `3` ;
2. faire avancer le moteur gauche à la vitesse `255` ;
3. faire avancer le moteur droit à la vitesse `255` ;
4. exécuter la séquence pendant `1000 ms`.

Le programme Arduino détecte le début et la fin de la séquence à partir des doubles crochets `[[ ... ]]`, puis la décode avant son exécution.

## Commandes disponibles

| Commande | Fonction                               |
| -------- | -------------------------------------- |
| `ns`     | Numéro de la séquence                  |
| `mga`    | Moteur gauche vers l'avant             |
| `mgr`    | Moteur gauche vers l'arrière           |
| `mda`    | Moteur droit vers l'avant              |
| `mdr`    | Moteur droit vers l'arrière            |
| `mst`    | Arrêt des moteurs                      |
| `t`      | Durée maximale d'exécution             |
| `ad1`    | Distance d'arrêt associée au capteur 1 |

Chaque commande est associée à un paramètre entier et est stockée dans une liste interne avant son exécution.

## Pilotage des moteurs

Les moteurs sont commandés individuellement grâce à des sorties PWM.

La vitesse est représentée par une valeur comprise entre `0` et `255`. Le sens de rotation est déterminé par une sortie numérique dédiée.

Le programme distingue ainsi :

* moteur gauche avant ;
* moteur gauche arrière ;
* moteur droit avant ;
* moteur droit arrière ;
* arrêt des deux moteurs.

## Capteurs de distance

Le robot utilise **deux capteurs VL53L1X** communiquant avec l'Arduino via **I²C**.

Les capteurs sont initialisés individuellement afin de leur attribuer des adresses différentes. Le programme utilise une fréquence I²C de `400 kHz` et configure les capteurs en mode de mesure courte distance.

Les mesures sont actualisées périodiquement et comprennent :

* distance du capteur 1 ;
* fiabilité de la mesure 1 ;
* distance du capteur 2 ;
* fiabilité de la mesure 2.

Les distances sont exprimées en millimètres.

## Sécurité et arrêt automatique

Une distance d'arrêt peut être définie avec la commande `ad1`.

À chaque boucle d'exécution, la distance du premier capteur est vérifiée. Si elle devient inférieure à la distance d'arrêt configurée, les moteurs sont arrêtés automatiquement.

```text
ad1 > distance_capteur_1
        │
        ▼
   Arrêt moteurs
```

Le programme possède également un **timeout de séquence**, fixé par défaut à `1000 ms`, permettant d'arrêter automatiquement les moteurs lorsque la durée maximale d'exécution est atteinte.

## Retour des données

À la fin de l'exécution d'une séquence, l'Arduino renvoie les informations sous forme de **JSON**.

Exemple de structure :

```json
{
  "ns": 3,
  "d1": 450,
  "f1": 12,
  "d2": 470,
  "f2": 11
}
```

Le retour contient notamment le numéro de séquence exécutée ainsi que les distances et indicateurs de fiabilité associés aux deux capteurs.

## Organisation du programme

Le programme Arduino est organisé autour de plusieurs fonctions :

```text
Initialisation
     │
     ├── Initialisation communication série
     ├── Initialisation moteurs
     └── Initialisation VL53L1X
             │
             ▼
        Boucle principale
             │
      ┌──────┴──────┐
      │             │
Lecture commandes   Lecture capteurs
      │             │
      ▼             ▼
Décodage         Surveillance distance
      │             │
      └──────┬──────┘
             ▼
      Exécution commandes
             │
             ▼
       Arrêt / Timeout
             │
             ▼
       Retour JSON
```

Le décodage transforme la séquence textuelle reçue en une liste interne contenant, pour chaque commande, son identifiant et son paramètre.

## Matériel

* Arduino UNO
* Châssis de robot mobile
* 4 moteurs
* Shield de commande moteurs DRI0009 2×2A
* Shield connecteurs I²C / E/S numériques
* Shield XBee
* 2 capteurs de distance VL53L1X
* Batteries rechargeables 9V
* Communication USB / XBee

## Technologies et notions

* Arduino
* Programmation embarquée
* C/C++
* PWM
* I²C
* Communication série
* XBee
* VL53L1X
* Commande de moteurs
* Parsing de protocole
* JSON
* Capteurs de distance
* Systèmes embarqués

## Contexte

**Projet collectif de robotique — Octobre 2023**

Travail réalisé autour de la programmation d'un robot mobile, de la commande des moteurs, de l'acquisition de mesures par capteurs et de la communication entre l'ordinateur et le robot.

### Projet Académique

Il s'agit d'un projet collectif réalisé à l'université en 2023.

## Licence MIT 
