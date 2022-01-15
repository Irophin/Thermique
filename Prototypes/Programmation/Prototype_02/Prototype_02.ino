//---Importation des libraries---//
#include "HX711.h"
#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <MsTimer2.h>

//----Declaration des entrées----//
#define DT A3
#define SK A2
#define Thermi_Chaud A0
#define Thermi_Froid A1
#define Capt_FinCourse 2
#define BoutonD 5
#define BoutonG 4

//----Declaration des sorties----//
#define Chauffage 10
#define Peltier 11
#define Direction 8
#define Step 7

//-Initialisation des composants-//
HX711 Balance;
LiquidCrystal_I2C lcd(0x27, 20, 3);
Adafruit_INA219 ina219;

//--Déclaration des constantes--//
bool Admin = true;

int Vitesse = 2000;
float Hauteur_max = 57;
float Pas_max = 1262;
float Poids_chariot = 3 ;
float Contrainte_max = 3;
float Surface = 0.0024;
float Consigne_Froid = 14;
float Consigne_Chaud = 23;
int Prechauffage = 120;
float Erreur_StatistiqueC = 0;
float Erreur_StatistiqueF = 1.5;
float Intervalle_Mesure = 50;

//--Déclaration des variables--//
float RapportDP = Hauteur_max / Pas_max;
float ElapsedTime, Time, TimePrev, Temps_Debut;
float Temp_Peltier, Temp_Resistance, Detla_Temp;
float Moyenne_Chaud, Moyenne_Froid = 0.0;
float Lambda, Resistance_th ;
float Energie, Energie_Total, Puissance, Puissance_Moyenne = 0.0;
float Loop, Epaisseur, Contrainte, Temps = 0.0;
int Temps_Mesure = 300;
int Menu = 1;
bool Verification, Timer;

void setup() {
  //----Declaration des entrées----//
  pinMode(DT, INPUT);
  pinMode(SK, INPUT);
  pinMode(Thermi_Froid, INPUT);
  pinMode(Thermi_Chaud, INPUT);
  pinMode(Capt_FinCourse, INPUT);
  pinMode(BoutonD, INPUT);
  pinMode(BoutonG, INPUT);

  //----Declaration des sorties----//
  pinMode(Chauffage, OUTPUT);
  pinMode(Peltier, OUTPUT);
  pinMode(Direction, OUTPUT);
  pinMode(Step, OUTPUT);

  //-Initialisation des composants-//
  Balance.begin(DT, SK);
  ina219.begin();
  lcd.backlight();
  lcd.init();
  lcd.clear();
  uint32_t currentFrequency;
  MsTimer2::set(Intervalle_Mesure, FCT_Timer);
  MsTimer2::stop();
  Serial.begin(9600);

  //---Confirmation utilisateur---//
  lcd.setCursor(1, 1);
  delay(500);
  lcd.print("Initialisation...");
  delay(2000);
}

void loop() {
  //Mise en place du chariot en position haute
  Initialisation();
  Epaisseur = Hauteur_max;

  //Appuyer pour demmarer
  while (digitalRead(BoutonD) == 1) {
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("Appuyer pour");
    lcd.setCursor(5, 2);
    lcd.print("Commencer");
    delay(300);
  } delay(1000);

  //Choix du temps de mesure
  while (digitalRead(BoutonG) == 1) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Mode de mesure :");
    lcd.setCursor(0, 3);
    lcd.print("OK");
    lcd.setCursor(18, 3);
    lcd.print(">>");

    if (digitalRead(BoutonD) == 0) {
      Menu++;
      if (Menu == 4) {
        Menu = 1;
      }
    }
    switch (Menu) {
      case 1:
        lcd.setCursor(3, 1);
        lcd.print("Mesure Rapide");
        lcd.setCursor(5, 2);
        lcd.print("< 5 Min >");
        Temps_Mesure = 300;
        break;

      case 2:
        lcd.setCursor(2, 1);
        lcd.print("Mesure Classique");
        lcd.setCursor(5, 2);
        lcd.print("< 10 Min >");
        Temps_Mesure = 600;
        break;

      case 3:
        lcd.setCursor(3, 1);
        lcd.print("Mesure Longue");
        lcd.setCursor(5, 2);
        lcd.print("< 15 Min >");
        Temps_Mesure = 900;
        break;

      default:
        Serial.println("Erreur mode inconnue ");
        break;
    } delay(300);
  } delay(1000);

  //Mise en place du chariot en position basse
  int Descente = 12;
  int Compteur_Pas = 0;
  Verification = false;
  while (Verification == false) {
    while (Contrainte <= Contrainte_max) {
      Compteur_Pas += Descente;
      Contrainte = abs(0.96875 * pow(10, -5) * Balance.read() - 1.1998) - Poids_chariot;
      Rotation(Descente, "BAS");
      Epaisseur -= RapportDP * Descente;
      lcd.clear();
      lcd.setCursor(1, 1);
      lcd.print("Descente en cours");
      lcd.setCursor(0, 2);
      lcd.print("Epaisseur : ");
      lcd.print(Epaisseur, 1);
      lcd.print(" mm");
      delay(300);

      if (Epaisseur <= 0 && Admin == false) {

        delay(1000);
        Serial.println("Etalonnage detecté");
        lcd.clear();
        lcd.setCursor(7, 0);
        lcd.print("Erreur");
        lcd.setCursor(1, 1);
        lcd.print("Vous devez placer");
        lcd.setCursor(3, 2);
        lcd.print("un echantillon");
        lcd.setCursor(14, 3);
        lcd.print("-Reset");

        while (1) {
          delay(30);
        }
      }
    } delay(500);
    Contrainte = abs(0.96875 * pow(10, -5) * Balance.read() - 1.1998) - Poids_chariot;
    if (Contrainte >= Contrainte_max) {
      Verification = true;
      Serial.println("Epaisseur de l'echantillon valider");
    }
  } Epaisseur /= 1000; delay(1000);

  Serial.print("Chariot en position Basse : "); Serial.print(Epaisseur*1000);
  Serial.print("mm d'épaisseur  | ");
  Serial.print(Compteur_Pas); Serial.print(" pas | Contrainte ");
  Serial.println(Contrainte);


  //Pré-chauffage
  Verification = false;
  Temps_Debut = millis();
  Temps = 0;
  while (Verification == false) {
    float Verif_Peltier = 0;
    float Verif_Resistance = 0;
    int Moyenne_Verif = 10;

    for (Loop = 0 ; Loop < Moyenne_Verif ; Loop++) {
      Asservissement();
      Verif_Peltier += Temp_Peltier;
      Verif_Resistance += Temp_Resistance;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Consigne Chaud "); lcd.print(Consigne_Chaud + Erreur_StatistiqueC, 1);
      lcd.setCursor(7, 1);
      lcd.print(Temp_Resistance); lcd.print(" C");
      lcd.setCursor(0, 2);
      lcd.print("Consigne Froid "); lcd.print(Consigne_Froid + Erreur_StatistiqueF, 1);
      lcd.setCursor(7, 3);
      lcd.print(Temp_Peltier); lcd.print(" C");
      delay(500);
    }
    Verif_Resistance /= Moyenne_Verif;
    Verif_Peltier /= Moyenne_Verif;
    Serial.print("Stabilisation chaud : "); Serial.print(Verif_Resistance); Serial.print(" °C || Stabilisation froid : "); Serial.print(Verif_Peltier); Serial.println(" °C ");
    Temps = (millis() - Temps_Debut)/1000;
    Serial.print("Préchauffage : "); Serial.print(Temps); Serial.println("s");
    float Marge_Erreur = 0.3;
    if ((Verif_Peltier < Consigne_Froid + Marge_Erreur + Erreur_StatistiqueF && Verif_Peltier > Consigne_Froid - Marge_Erreur + Erreur_StatistiqueF ) && (Verif_Resistance < Consigne_Chaud + Marge_Erreur + Erreur_StatistiqueC && Verif_Resistance > Consigne_Chaud - Marge_Erreur + Erreur_StatistiqueC) && (Temps>Prechauffage)) {
      Verification = true;
      Serial.println("Demarage de la mesure");
    }
  } delay(1000);

  //Mesure de la resistance thermique
  Energie_Total = 0;
  Loop = 1;
  Timer = false;
  int Compteur = 0;
  Temps_Debut = millis();
  Temps=0;
  MsTimer2::start();
  while (Temps <= Temps_Mesure) {
    if (Compteur <= 20000) {
      if (Timer == true) {
        Timer=false;
        Puissance = ina219.getPower_mW();
        Energie = Puissance * Intervalle_Mesure;
        Energie_Total += Energie;
      }
      if (Compteur % 500 == 1) {
        Asservissement();
      }

    } else {
      Serial.print("Temps : ");Serial.print(Temps);Serial.print("s, sur :");Serial.print(Temps_Mesure);Serial.println(" s");
      Compteur = 0;
      Moyenne_Froid += Temp_Peltier;  Moyenne_Chaud += Temp_Resistance;
      Detla_Temp = (Moyenne_Chaud - Moyenne_Froid) / Loop ;
      Puissance_Moyenne = Energie_Total*pow(10,-6) / Temps;
      Lambda = (Puissance_Moyenne * Epaisseur) / (Surface * Detla_Temp);
      Resistance_th = Epaisseur / Lambda;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("T_F="); lcd.print(Temp_Peltier, 1); lcd.print("C");

      lcd.setCursor(10, 0);
      lcd.print("T_C="); lcd.print(Temp_Resistance, 1); lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print("E:"); lcd.print(Energie_Total*pow(10,-6), 1); lcd.print("J");

      lcd.setCursor(10, 1);
      lcd.print("Pm:"); lcd.print(Puissance_Moyenne, 2); lcd.print("W");

      lcd.setCursor(0, 2);
      lcd.print("lda:"); lcd.print(Lambda, 3);
      
      lcd.setCursor(10, 2);
      lcd.print("Rth:"); lcd.print(Resistance_th, 3);


      lcd.setCursor(0, 3);
      int Avancement = Temps / Temps_Mesure * 20;
      for (int x = 0; x < Avancement; x++) {
        lcd.print("*");
      }
      Loop++;
      Temps = (millis() - Temps_Debut) / 1000;
    } Compteur++;
  } MsTimer2::stop(); delay(3000);

  Initialisation();
  Epaisseur *= 1000;

  Serial.println(" Fin des mesures : Affichage des résultats :");

  Serial.print("Delta Temperature : "); Serial.println(Detla_Temp,2);
  Serial.print("Puissance Absorbée : "); Serial.println(Puissance_Moyenne, 2);
  Serial.print("Epaisseur : "); Serial.println(Epaisseur, 2);
  Serial.print("Flux thermique : "); Serial.println(Lambda, 4);
  Serial.print("Resistance thermique : "); Serial.println(Resistance_th, 4);

  lcd.clear();

  //Affichage des résultats
  while (digitalRead(BoutonD) == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Puissance : "); lcd.print(Puissance_Moyenne, 2); lcd.print(" W");

    lcd.setCursor(0, 1);
    lcd.print("Epaisseur :"); lcd.print(Epaisseur, 2); lcd.print(" mm");

    lcd.setCursor(0, 2);
    lcd.print("Conduction : "); lcd.print(Lambda, 4);

    lcd.setCursor(0, 3); lcd.print("Rth : "); lcd.print(Resistance_th, 4);
  } delay(2000);

}

//Fonction permettant de mettre le chariot en possition haute
void Initialisation() {
  Serial.println("Initialisation en cours ");
  analogWrite(Chauffage, 0);
  analogWrite(Peltier, 0);
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Montee du chariot");
  lcd.setCursor(4, 2);
  lcd.print("En cours ...");

  while (digitalRead(Capt_FinCourse) == 0) {
    Rotation(10, "HAUT");
  }
  Rotation(70, "BAS");
  delay(200);
  while (digitalRead(Capt_FinCourse) == 0) {
    Rotation(1, "HAUT");
  }

  Serial.println("Chariot en position haute");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Chariot en position");
  lcd.setCursor(7, 2);
  lcd.print("haute");
}

//Fonction permettant de déclancher des pas
void Rotation(int Pas, String Dir) {
  if (Dir == "HAUT") {
    digitalWrite(Direction, LOW);
  }
  else if (Dir == "BAS") {
    digitalWrite(Direction, HIGH);
  }
  else {
    Serial.println("Erreur de direction moteur");
  }

  for (Loop = 0 ; Loop <= Pas; Loop++) {
    digitalWrite(Step, HIGH);
    delayMicroseconds(Vitesse);
    digitalWrite(Step, LOW);
    delayMicroseconds(Vitesse);
  }
}

//Fonction d'aiguillage Thermistance
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

//Fonction Thermometre
float Capteur_temperature(int Thermistance) {
  int Compte_Tours = 100;
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

//Fonction Asservissement
float PID_errorC, Previous_errorC = 0;
float PID_errorF, Previous_errorF = 0;
int PID_valueC = 0, PID_valueF = 0;

int kpC = 62 ;   float kiC = 0.3;   int kdC = 1.2;   //Constante PID Resistance
int kpF = 235;   float kiF = 0.6;   int kdF = 1.2;   //Constante PID Froid

int PID_pC, PID_iC, PID_dC = 0;
int PID_pF, PID_iF, PID_dF = 0;

void Asservissement() {
  Temp_Peltier = Thermometre("Peltier") ;
  Temp_Resistance = Thermometre("Resistance") ;

  TimePrev = Time;
  Time = millis();
  ElapsedTime = (Time - TimePrev) / 1000;

  //PID Resistance
  PID_errorC = Consigne_Chaud - Temp_Resistance;
  PID_pC = kpC * PID_errorC;
  if (1.0 > PID_errorC > 0)  {
    PID_iC = PID_iC + (kiC * PID_errorC);
  }
  else if ( 1.0 < PID_errorC ) {
    PID_iC = 0;
  }
  PID_dC = kdC * ((PID_errorC - Previous_errorC) / ElapsedTime);
  PID_valueC = PID_pC + PID_iC + PID_dC;

  Previous_errorC = PID_errorC;

  if (PID_valueC < 0)    {
    PID_valueC = 0;
  }
  if (PID_valueC > 255)  {
    PID_valueC = 255;
  }

  //PID Peltier
  PID_errorF = Temp_Peltier - Consigne_Froid ;
  PID_pF = kpF * PID_errorF;
  if (1.0 > PID_errorF > 0)  {
    PID_iF = PID_iF + (kiF * PID_errorF);
  }
  else if (  1.0 < PID_errorF) {
    PID_iF = 0;
  }
  PID_dF = kdF * ((PID_errorF - Previous_errorF) / ElapsedTime);
  PID_valueF = PID_pF + PID_iF + PID_dF;

  Previous_errorF = PID_errorF;

  if (PID_valueF < 0)    {
    PID_valueF = 0;
  }
  if (PID_valueF > 255)  {
    PID_valueF = 255;
  }
  //Serial.print("Peltier : "); Serial.print(PID_valueF); Serial.print("   "); Serial.println(Temp_Peltier);
  //Serial.print("Resistance : "); Serial.print(PID_valueC); Serial.print("   "); Serial.println(Temp_Resistance);
  //Serial.println("--");

  analogWrite(Chauffage, PID_valueC);
  analogWrite(Peltier, PID_valueF);
}

void FCT_Timer() {
  Timer = true;
}
