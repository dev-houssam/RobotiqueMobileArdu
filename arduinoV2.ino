

// codes des commandes
// on reçoit les commandes dans une ligne de texte

#define ERREUR 0

#define NUM_SEQ_CMD 1		// numéro de la séquence de commandes

#define MOTEUR_G_AV 2		// moteur gauche vers l'avant
#define MOTEUR_G_AR 3		// moteur gauche vers l'arrière
#define MOTEUR_D_AV 4		// moteur droit vers l'avant
#define MOTEUR_D_AR 5		// moteur droit vers l'arrière
#define MOTEURS_STOP 6		// arrêt des moteurs
#define MOTEURS_TIMEOUT 7	// timeout pour arrêt des moteurs
#define AD1 8 // arret par rapport a d1 (distance)
#define AD2 8 // arret par rapport a d2 (distance)


void init_buf_in();

void moteurs_init();
void moteur_gauche_avant(int);
void moteur_droit_avant(int);
void moteur_gauche_arriere(int);
void moteur_droit_arriere(int);
void moteurs_stop();
int getd1();
int getd2();


void vl53l1x_init();
void vl53l1x_read();
void vl53l1x_json();

#define BUF_IN 200	// taille max du buffer de lecture
#define LCMD 50		// taille max du tableau de la liste de commandes
#define TIMEOUT 1000	// durée max en ms d'une séquence de commandes

char buf_in[BUF_IN];	// buffer de lecture
int buf_in_count = 0;	// taille courante du buffer

int lcmd[LCMD]; // tableau de la liste des commandes à exécuter (2 entiers par commande)
int lcmd_count = 0;	// taille courante du tableau de la liste des commandes
int lcmd_pos = 0;	// position de la commande courante en cours d'exécution

// données d'exécution

int exec_en_cours = 0;	// à 1 si exécution d'une séquence de commandes en cours

unsigned long time = 0;	// on arrête l'exécution si time + timeout > millis()
unsigned long timeout = TIMEOUT;	// durée max en ms d'une séquence de commandes
int ad1 = 0; // distance d'arrêt 
int ad2 = 0; // distance d'arrêt 
// pour arrêter l'exécution d'une séquence de commandes on met timeout à 0

int num_seq_cmd = -1;	// numéro de la séquence de commandes


// [[ns 3][mga 255][mda 255][t 1000]]

void setup() {

	init_buf_in();

	Serial.begin(9600);

	vl53l1x_init();

	delay(2000);

	Serial.println("OK Arduino");

	
}

int c = -1;		// caractère courant lu
int c_prec = -1;	// caractère précédent lu

void loop() {

	if (0 == 1) return;

	vl53l1x_read();
  if (ad1 > getd1()){
    moteurs_stop();
  }
  if (ad2 > getd2()){
    moteurs_stop();
  }

	if (Serial.available()) {
		c_prec = c;
		c = Serial.read();
		if ((c == '[') && (c_prec == '[')) {

			// nouvelle lecture d'une séquence de commandes

			init_buf_in();
			buf_in[buf_in_count++] = '[';
		}
		else if ((c == ']') && (c_prec == ']')) {
			// fin lecture, nouvelle séquence de commandes reçue
			// Serial.print("lu = "); Serial.println(buf_in);
			decoder_seq_commandes();

			init_donness_execution();
	
			exec_en_cours = 1;
		}
		else if (c == '\n') {
		}
		else {
			buf_in[buf_in_count++] = c;
		}
	}

	if (exec_en_cours == 1) {
		if (time + timeout > millis()) {
			exec_cmd();
		}
		else {
			moteurs_stop();
			// on produit le résultat JSON

			// numéro de la séquence exécutée
			Serial.print("{\"ns\":");
			Serial.print(num_seq_cmd);

			// distances et fiabilités des distances
			Serial.print(",");
			vl53l1x_json();

			Serial.println("}");

			exec_en_cours = 0;
		}

	}


}


/*******************************************************************************/
/********************* décodage de la séquence de commandes ********************/
/*******************************************************************************/

void decoder_seq_commandes() {
	// analyse de la séquence de commandes reçue dans buf_in
	// résultat dans lcmd (2 entiers par commande, numéro commande et paramètre)
	
	for (int i=0 ; i<LCMD ; i++) {
		lcmd[i] = ERREUR; // init lcmd à 0
	}
	lcmd_count = 0;

	int pos = 0;
	int p1;
	for (;;) { // décodage d'une commande à chaque itération
		if (pos >= buf_in_count) break;

		if (buf_in[pos] == '[') {
			pos++;
			if ((buf_in[pos] == 'n') && (buf_in[pos+1] == 's')) {
				lcmd[lcmd_count++] = NUM_SEQ_CMD;
				pos += 2;
			}
			else if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 'g') && (buf_in[pos+2] == 'a')) {
				lcmd[lcmd_count++] = MOTEUR_G_AV;
				pos += 3;
			}
			else if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 'g') && (buf_in[pos+2] == 'r')) {
				lcmd[lcmd_count++] = MOTEUR_G_AR;
				pos += 3;
			}
			else if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 'd') && (buf_in[pos+2] == 'a')) {
				lcmd[lcmd_count++] = MOTEUR_D_AV;
				pos += 3;
			}
			else if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 'd') && (buf_in[pos+2] == 'r')) {
				lcmd[lcmd_count++] = MOTEUR_D_AR;
				pos += 3;
			}
			else if ((buf_in[pos] == 'm') && (buf_in[pos+1] == 's') && (buf_in[pos+2] == 't')) {
				lcmd[lcmd_count++] = MOTEURS_STOP;
				pos += 3;
			}
     else if ((buf_in[pos] == 'a') && (buf_in[pos+1] == 'd') && (buf_in[pos+2] == '1')) {
       lcmd[lcmd_count++] = AD1;
        pos += 3;
      }
       else if ((buf_in[pos] == 'a') && (buf_in[pos+1] == 'd') && (buf_in[pos+2] == '2')) {
       lcmd[lcmd_count++] = AD2;
        pos += 3;
      }
			else if (buf_in[pos] == 't') {
				lcmd[lcmd_count++] = MOTEURS_TIMEOUT;
				pos++;
			}
			else {
				lcmd[lcmd_count++] = ERREUR;
				break;
			}

			if (buf_in[pos] != ' ') {
				lcmd[lcmd_count++] = ERREUR;
				break;
			}
			pos++;
			if ((buf_in[pos] < '0') || (buf_in[pos] > '9')) {
				lcmd[lcmd_count++] = ERREUR;
				break;
			}
			p1 = pos; // début du paramètre (entier)
			for (;;) {
				pos++;
				if ((buf_in[pos] < '0') || (buf_in[pos] > '9')) {
					break;
				}
			}
			if (buf_in[pos] == ']') {
				buf_in[pos] = 0;
				lcmd[lcmd_count++] = atoi(buf_in+p1);
				pos++;
			}
			else {
				lcmd[lcmd_count-1] = ERREUR;
				break;
			}
			


		}
		else {
			lcmd[lcmd_count++] = ERREUR;
			break;
		}
	}



	// affichage du tableau des commandes
	// for (int i=0 ; i<lcmd_count ; i+=2) {
	// 	Serial.print("cmd = ");
	// 	Serial.print(lcmd[i]);
	// 	Serial.print(" ");
	// 	Serial.print(lcmd[i+1]);
	// 	Serial.println();
	// }

}


/*******************************************************************/
/********************* exécution d'une commande ********************/
/*******************************************************************/

void exec_cmd() {
	if (lcmd_pos >= lcmd_count) return;

	int cmd = lcmd[lcmd_pos++];
	int param = lcmd[lcmd_pos++];

	// Serial.print("cmd = "); Serial.print(cmd); Serial.print(" param = "); Serial.println(param);

	if (cmd == NUM_SEQ_CMD) {
		num_seq_cmd = param;
	}
	else if (cmd == MOTEUR_G_AV) moteur_gauche_avant(param);
	else if (cmd == MOTEUR_D_AV) moteur_droit_avant(param);
	else if (cmd == MOTEUR_G_AR) moteur_gauche_arriere(param);
	else if (cmd == MOTEUR_D_AR) moteur_droit_arriere(param);
	else if (cmd == MOTEURS_STOP) moteurs_stop();
	else if (cmd == MOTEURS_TIMEOUT) timeout = param;
  else if (cmd == AD1) ad1 = param;
  else if (cmd == AD2) ad2 = param;

}

/*******************************************************************/
/************************** init_buf_in ****************************/
/*******************************************************************/

void init_buf_in() {
	for (int i=0 ; i<BUF_IN ; i++) {
		buf_in[i] = 0;
	}
	buf_in_count = 0;
}

/*******************************************************************/
/************************** init_donness_execution *****************/
/*******************************************************************/

void init_donness_execution() {
	lcmd_pos = 0;	// commande courante

	num_seq_cmd = -1;

	time = millis();	// temps courant en ms
	timeout = TIMEOUT;	// durée de l'exécution d'une séquence de commandes

}

/*******************************************************************/
/********************* commande des moteurs ************************/
/*******************************************************************/

// Attention : suivant les cartes, on utilise l'une série de constantes ou l'autre

// https://www.gotronic.fr/art-shield-motor-2-x-2-a-dri0009-19345.htm
int E1 = 5;	// D5 vitesse moteur 1
int M1 = 4;	// D4 sens moteur 1
int E2 = 6;	// D6 vitesse moteur 2
int M2 = 7;	// D7 sens moteur 2

// https://fr.aliexpress.com/item/32786304281.html
// int E1 = 10;	// vitesse moteur 1
// int M1 = 12;	// sens moteur 1
// int E2 = 11;	// vitesse moteur 2
// int M2 = 13;	// sens moteur 2

int vitesse_gauche = 0;	// valeur de 0 à 255 (en fait de 100 ou 150 à 255), positif si vers l'avant, négatif si vers l'arrière
int vitesse_droit = 0;	// valeur de 0 à 255 (en fait de 100 ou 150 à 255)

void moteurs_init() {
	pinMode(M1, OUTPUT);   
	pinMode(M2, OUTPUT); 
	moteurs_stop();
}

void moteur_gauche_avant(int speed) {
  	vitesse_gauche = speed;
	digitalWrite(M1,LOW);
	analogWrite(E1, speed);   //PWM Speed Control
}

void moteur_gauche_arriere(int speed) {
	vitesse_gauche = -speed;
	digitalWrite(M1,HIGH);
	analogWrite(E1, speed);   //PWM Speed Control
}

void moteur_droit_avant(int speed) {
	vitesse_droit = speed;
 	digitalWrite(M2,LOW);
	analogWrite(E2, speed);   //PWM Speed Control
}

void moteur_droit_arriere(int speed) {
	vitesse_droit = -speed;
	digitalWrite(M2,HIGH);
	analogWrite(E2, speed);   //PWM Speed Control
}

void moteurs_stop() {
	analogWrite(E1, 0);   //PWM Speed Control
	analogWrite(E2, 0);   //PWM Speed Control
	vitesse_gauche = 0;
	vitesse_droit = 0;
}



/*******************************************************************/
/******************* VL53L1X Capteurs de distance ******************/
/*******************************************************************/


// utilise la librairie vl53l1x-arduino-master
// https://www.arduino.cc/reference/en/libraries/vl53l1x/

#include <Wire.h>
#include <VL53L1X.h>

#define SENSOR_COUNT 2	// 2 capteurs de distance

#define SENSOR_TIMEOUT 100	// une lecture toutes les 100 ms

// The number of sensors in your system.
int sensorCount = SENSOR_COUNT; // on pourra fonctionner avec 0, 2 ou 4 capteurs

// The Arduino pin connected to the XSHUT pin of each sensor.
const uint8_t xshutPins[SENSOR_COUNT] = { 2,3 };
//const uint8_t xshutPins[SENSOR_COUNT] = { 2,3, 8,9 };

VL53L1X sensors[SENSOR_COUNT];

//int dist[SENSOR_COUNT+SENSOR_COUNT]; // tableau des distances, 8 valeurs : 4 x (distance en mm + fiabilité mesure)

unsigned long time_sensor = 0;		// si 0 : timeout d'arrêt des moteurs 

int vl53l1x_dist_1 = -1;	// distance capteur 1 en mm
int vl53l1x_fiabilite_1 = -1;	// fiabilité capteur 1
int vl53l1x_dist_2 = -1;	// distance capteur 2 en mm
int vl53l1x_fiabilite_2 = -1;	// fiabilité capteur 2

int getd1(){
  return vl53l1x_dist_1;
}
int getd2(){
  return vl53l1x_dist_2;
}

void vl53l1x_init() {
	Wire.begin();
	Wire.setClock(400000); // use 400 kHz I2C

    // Disable/reset all sensors by driving their XSHUT pins low.
    for (uint8_t i = 0; i < SENSOR_COUNT; i++)
    {
        pinMode(xshutPins[i], OUTPUT);
        digitalWrite(xshutPins[i], LOW);
    }

  // Enable, initialize, and start each sensor, one by one.
  for (uint8_t i = 0; i < SENSOR_COUNT; i++)
    {
        // Stop driving this sensor's XSHUT low. This should allow the carrier
        // board to pull it high. (We do NOT want to drive XSHUT high since it is
        // not level shifted.) Then wait a bit for the sensor to start up.
        pinMode(xshutPins[i], INPUT);
        delay(10);

        sensors[i].setTimeout(500);
        if (!sensors[i].init()) {	// problème : blocage si pas de capteur connecté
            Serial.print("Capteur de distance ");
            Serial.print(i);
            Serial.println(" non trouvé");
            sensorCount = i;
            break;
        }

        // Each sensor must have its address changed to a unique value other than
        // the default of 0x29 (except for the last one, which could be left at
        // the default). To make it simple, we'll just count up from 0x2A.
        sensors[i].setAddress(0x2A + i);

        sensors[i].startContinuous(50);
        //sensors[i].setDistanceMode(VL53L1X::Long);
        //sensors[i].setDistanceMode(VL53L1X::Medium);
        sensors[i].setDistanceMode(VL53L1X::Short);

        time_sensor = millis();
    }

	Serial.print("Capteurs de distance trouvés : ");
	Serial.print(sensorCount);

	if (sensorCount > 0) Serial.println(" (initialisation en mode court)");
	else Serial.println();


}

void vl53l1x_read() {
    if (time_sensor+ SENSOR_TIMEOUT < millis()) { // lecture des capteurs de distance
		
		time_sensor = millis();

        // int pos = 0;
        // for (uint8_t i = 0; i < sensorCount; i++) {
        //     dist[pos++] = sensors[i].read();	// distance en mm
        //     dist[pos++] = (int)sensors[i].ranging_data.peak_signal_count_rate_MCPS; // fiabilité de la mesure
        // }

        vl53l1x_dist_1 = sensors[0].read();	// distance en mm
        vl53l1x_fiabilite_1 = sensors[0].ranging_data.peak_signal_count_rate_MCPS; // fiabilité de la mesure

        vl53l1x_dist_2 = sensors[1].read();	// distance en mm
        vl53l1x_fiabilite_2 = sensors[1].ranging_data.peak_signal_count_rate_MCPS; // fiabilité de la mesure


    }
}

void vl53l1x_json() {
    //Serial.print("{");

    Serial.print("\"d1\":");
    Serial.print(vl53l1x_dist_1);
    Serial.print(",\"f1\":");
    Serial.print(vl53l1x_fiabilite_1);
    Serial.print(", \"d2\":");
    Serial.print(vl53l1x_dist_2);
    Serial.print(",\"f2\":");
    Serial.print(vl53l1x_fiabilite_2);

    //Serial.println("}");
}
