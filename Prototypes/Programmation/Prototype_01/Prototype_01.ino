//---Importation des libraries---//
#include "HX711.h"
#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//----Declaration des entrees----//
#define DT A4
#define SK A6
#define Thermi_Chaud A0
#define Thermi_Froid A1
#define Capt_FinCourse 51
#define Bouton 30

//----Declaration des sorties----//
#define Chauffage 47
#define Peltier 49
#define Direction 13
#define Step 12

//---Declaration des variables---//
HX711 Balance;
LiquidCrystal_I2C lcd(0x27 , 20, 3);
Adafruit_INA219 ina219;
int Vitesse = 2000;
float Epaisseur;

float Hauteur_max = 59.0;
float Pas_max = 1495;
float Donnees[2];
float Etalonnage_contrainte = 0 ;

int Consigne_Froid = 15;
int Consigne_Chaud = 40;

void setup() {
  pinMode(DT, INPUT);
  pinMode(SK, INPUT);
  pinMode(Thermi_Froid, INPUT);
  pinMode(Thermi_Chaud, INPUT);
  pinMode(Capt_FinCourse, INPUT);
  pinMode(Bouton, INPUT);

  pinMode(Chauffage, OUTPUT);
  pinMode(Peltier, OUTPUT);
  pinMode(Direction, OUTPUT);
  pinMode(Step, OUTPUT);

  Balance.begin(DT, SK);
  ina219.begin();
  lcd.backlight();
  lcd.init();
  lcd.clear();

  uint32_t currentFrequency;
  Serial.begin(9600);
  lcd.setCursor(1, 1);
  delay(1000);
  lcd.print("Initialisation...");
  delay(2000);

}

void loop() {

  Initialisation();
  Epaisseur = Hauteur_max;

  lcd.clear();
  while (digitalRead(Bouton) == 0) {
    lcd.setCursor(3, 1);
    lcd.print("Appuyer pour");
    lcd.setCursor(5, 2);
    lcd.print("demarrer");
    delay(20);
  }

  long Mesure = 0;
  float D_pas = Hauteur_max / Pas_max;
  int Pas = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Descente du chariot");
  lcd.setCursor(4, 1);
  lcd.print("En cours ...");
  while (Mesure < 200000) {
    Mesure = (abs(Balance.read())- Etalonnage_contrainte)*(-1);

    Serial.print("Contrainte : ");
    Serial.println(Mesure);


    for (int Loop = 0; Loop < 5; Loop++) {
      digitalWrite(Step, HIGH);
      delayMicroseconds(Vitesse);
      digitalWrite(Step, LOW);
      delayMicroseconds(Vitesse);
      Epaisseur -= D_pas;
      Pas++;
    }
    lcd.setCursor(0, 3);
    lcd.print("Epaisseur : ");
    lcd.print(Epaisseur, 1);
    lcd.print("mm ");

  }

  Serial.print("Chariot en position Basse : ");
  Serial.print(Epaisseur);
  Serial.print("mm d'épaisseur  | ");
  Serial.print(Pas);
  Serial.print(" pas | Contrainte ");
  Serial.println(Mesure);

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Chariot en position");
  lcd.setCursor(0, 2);
  lcd.print("Epaisseur : ");
  lcd.print(Epaisseur, 1);
  lcd.print("mm");

  delay(5000);
  Epaisseur = Epaisseur / 1000;

  bool Ready = false;
  while (Ready == false) {
    float Verif_Chaud = 0;
    float Verif_Froid = 0;
    int Moyenne = 20;
    for (int loop = 0 ; loop < Moyenne ; loop++) {

      for (int Doublage = 0; Doublage < 2 ; Doublage++) {
        Asservissement(Donnees, -0.5);
        delay(500);
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Consigne Chaud ");
      lcd.print(Consigne_Chaud);
      lcd.print(" C");
      lcd.setCursor(7, 1);
      lcd.print(Donnees[1]);
      lcd.print(" C");
      lcd.setCursor(0, 2);
      lcd.print("Consigne Froid ");
      lcd.print(Consigne_Froid);
      lcd.print(" C");
      lcd.setCursor(7, 3);
      lcd.print(Donnees[0]);
      lcd.print(" C");


      Verif_Froid += Donnees[0];
      Verif_Chaud += Donnees[1];
      delay(500);
    }
    Verif_Chaud = Verif_Chaud / Moyenne;
    Verif_Froid = Verif_Froid / Moyenne;
    Serial.print("Stabilisation chaud : ");
    Serial.print(Verif_Chaud);
    Serial.print(" °C || Stabilisation froid : ");
    Serial.print(Verif_Froid);
    Serial.println(" °C ");

    if ((Verif_Froid < Consigne_Froid + 1 && Verif_Froid > Consigne_Froid - 1) && (Verif_Chaud < Consigne_Chaud + 1 && Verif_Chaud > Consigne_Chaud - 1)) {
      Ready = true;
      Serial.println("Demarage de la mesure");
    }
  }

  float Energie = 0;
  float Puissance = 0;
  float Moyenne_Chaud = 0;
  float Moyenne_Froid = 0;
  float Moyenne_Puissance;
  float Lambda = 0;
  float Detla_Temp;
  float Resistance_th ;
  float Dissipation = 1.1;

  int Temps_Mesure = 600;
  for (int Temps = 0; Temps < Temps_Mesure; Temps++) {
    lcd.clear();

    Asservissement(Donnees, 0);
    Moyenne_Froid += Donnees[0];
    Moyenne_Chaud += Donnees[1];
    Puissance = Donnees[2];

    if (Puissance > 0) {
      Energie += Puissance;
    }
    Moyenne_Puissance = Energie / Temps - Dissipation * Temps / Temps_Mesure;

    lcd.setCursor(0, 0);
    lcd.print("T_F=");
    lcd.print(Donnees[0], 1);
    lcd.print("C");

    lcd.setCursor(10, 0);
    lcd.print("T_C=");
    lcd.print(Donnees[1], 1);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Puissance : ");
    lcd.print(Puissance, 1);
    lcd.print(" W");

    lcd.setCursor(0, 2);
    lcd.print("Pm : ");
    lcd.print(Moyenne_Puissance, 3);
    lcd.print(" W");

    lcd.setCursor(0, 3);
    int Boucle = float(Temps) / Temps_Mesure * 20;
    for (int x = 0; x < Boucle + 1; x++) {
      lcd.print("*");
    }
    delay(860);
    Serial.println("Verification temps");

  }
  Moyenne_Puissance = Energie / Temps_Mesure - Dissipation;
  Moyenne_Froid = Moyenne_Froid / Temps_Mesure;
  Moyenne_Chaud = Moyenne_Chaud / Temps_Mesure;
  Detla_Temp = Moyenne_Chaud - Moyenne_Froid;
  Lambda = (Moyenne_Puissance * Epaisseur) / (0.0025 * Detla_Temp);
  Resistance_th = Epaisseur / Lambda;
  
  Initialisation();
  lcd.clear();

  Serial.println(" Fin des mesures : Affichage des résultats ");

  Serial.print("Temperature Froide : ");
  Serial.print(Moyenne_Froid);

  Serial.print(" | Temperature Chaude : ");
  Serial.println(Moyenne_Chaud);

  Serial.print("Puissance Absorbée : ");
  Serial.println(Moyenne_Puissance,2);

  Serial.print("Epaisseur : ");
  Serial.println(Epaisseur,2);

  Serial.print("Flux thermique : ");
  Serial.println(Lambda,4);

  Serial.print("Resistance thermique : ");
  Serial.println(Resistance_th,4);

  Epaisseur = Epaisseur * 1000;
  while (digitalRead(Bouton) == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Puissance : ");
    lcd.print(Moyenne_Puissance, 2);
    lcd.print(" W");

    lcd.setCursor(0, 1);
    lcd.print("Epaisseur :");
    lcd.print(Epaisseur, 2);
    lcd.print(" mm");

    lcd.setCursor(0, 2);
    lcd.print("Conduction : ");
    lcd.print(Lambda, 4);

    lcd.setCursor(0, 3);
    lcd.print("Rth : ");
    lcd.print(Resistance_th, 4);

  }
  delay(2000);
}

void Asservissement(float * Data, int Inertie) {
  float Temp_Peltier = Thermometre("Peltier") ;
  float Temp_Resistance = Thermometre("Resistance") ;

  if (Temp_Peltier >= Consigne_Froid) {
    digitalWrite(Peltier, HIGH);
  } else {
    digitalWrite(Peltier, LOW);
  }
  if (Temp_Resistance <= Consigne_Chaud + Inertie) {
    digitalWrite(Chauffage, HIGH);
  } else {
    digitalWrite(Chauffage, LOW);
  }
  float Puissance = abs(ina219.getPower_mW() / 1000);
  Data[0] = Temp_Peltier;
  Data[1] = Temp_Resistance;
  Data[2] = Puissance;
}

float Capteur_temperature(int Thermistance) {
  int Compte_Tours = 50;
  float Tension = 0;
  float Temperature = 0;
  float Resistance = 0;

  for (int Loop = 0 ; Loop < Compte_Tours ; Loop++) {

    Tension = analogRead(Thermistance) * 5.00 / 1023.00 ;
    Resistance = Tension * 10 / (5 - Tension);
    Temperature += 39.077 * exp(-(Resistance - 5.08) / 9.802) + 1.294;

  }
  float MoyenneTemperature = Temperature / Compte_Tours;
  return MoyenneTemperature ;
}

float Thermometre(String Demande) {
  float Temperature = 0;
  if (Demande == "Peltier") {
    Temperature = Capteur_temperature(Thermi_Froid);
    //Serial.print("Temperature peltier : "); Serial.println(Temperature);
    return Temperature;

  }
  else if (Demande == "Resistance") {
    Temperature = Capteur_temperature(Thermi_Chaud);
    //Serial.print("Temperature résistance : "); Serial.println(Temperature);
    return Temperature;
  }
  else {
    Serial.println("Erreur de la mesure de temperature");
    return Temperature;
  }
}

void Initialisation() {

  Serial.println("Initialisation en cours ");
  digitalWrite(Chauffage, LOW);
  digitalWrite(Peltier, LOW);
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Montee du chariot");
  lcd.setCursor(4, 2);
  lcd.print("En cours ...");

  digitalWrite(Direction, LOW);
  while (digitalRead(Capt_FinCourse) == 1) {
    digitalWrite(Step, HIGH);
    delayMicroseconds(Vitesse);
    digitalWrite(Step, LOW);
    delayMicroseconds(Vitesse);
  }
  Serial.println("Chariot en position haute");
  digitalWrite(Direction, HIGH);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Chariot en position");
  lcd.setCursor(7, 2);
  lcd.print("haute");
  digitalWrite(Direction, HIGH);
  delay(1000);
  Etalonnage_contrainte = abs(Balance.read());
  delay(1000);
}
