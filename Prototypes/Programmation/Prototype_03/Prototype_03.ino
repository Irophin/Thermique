#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <MsTimer2.h>

//----Declaration des entrées----//
// Thermometres
#define Thermi_Chaud A2
#define Thermi_Froid A1
#define Thermi_Ambiant A0
// Encodeur
#define Bouton 2
#define DT 3
#define CLK 4

//----Declaration des sorties----//
//Resistance Chauffante
#define Chauffage 9
//Plaque de peltier
#define Peltier 10

//-Initialisation des composants-//
//Ecran LCD
LiquidCrystal_I2C lcd(0x27, 20, 3);
//Module INA219
Adafruit_INA219 ina219;

//--Déclaration des constantes--//
float Surface = 0.0025;
float Consigne_Froid = 15;
float Consigne_Chaud;
int Prechauffage = 45;
float Intervalle_Mesure = 50;

//--Déclaration des variables--//
bool Verification;
bool Timer;

float Loop, Epaisseur, Temps = 0.0;
float Energie = 0.0;
float Energie_Total = 0.0;
float Puissance = 0.0;
float Puissance_Moyenne = 0.0;
float ElapsedTime, Time, TimePrev, Temps_Debut;
float Temp_Peltier, Temp_Resistance, Detla_Temp;
float Moyenne_Chaud = 0.0;
float Moyenne_Froid = 0.0;
float Lambda, Resistance_th ;
int Etat_DT, DerniereEtat_DT;
int Compteur;
int Menu;

//------Valeur par defaut------//
float Temp_Ambiant = 22;
int Temps_Mesure = 600;


void setup() {
  //----Declaration des entrées----//
  pinMode(Thermi_Froid, INPUT);
  pinMode(Thermi_Chaud, INPUT);
  pinMode(Thermi_Ambiant, INPUT);

  pinMode(DT, INPUT);
  pinMode(CLK, INPUT);
  pinMode(Bouton, INPUT_PULLUP);

  //----Declaration des sorties----//
  pinMode(Chauffage, OUTPUT);
  pinMode(Peltier, OUTPUT);

  //-Initialisation des composants-//
  ina219.begin();
  uint32_t currentFrequency;
  ina219.setCalibration_16V_400mA();

  DerniereEtat_DT = digitalRead(CLK);
  attachInterrupt(digitalPinToInterrupt(DT), ILS_Rotation, HIGH);

  MsTimer2::set(Intervalle_Mesure, FCT_Timer);
  MsTimer2::stop();

  lcd.backlight();
  lcd.init();
  lcd.clear();

  Serial.begin(9600);

  //---Confirmation utilisateur---//
  delay(200);
  lcd.setCursor(1, 0); lcd.print("Initialisation");
  lcd.setCursor(3, 1); lcd.print("-Thermick-");
  delay(200);
  Serial.println("Systeme pret :");
  while (digitalRead(Bouton) == HIGH) {
    delay(1);
  } delay(300); lcd.clear();

}



void loop() {
  Initialisation();

  //Gestion de l'epaisseur
  float Derniere_Epaisseur = -1;
  while (digitalRead(Bouton) == HIGH) {
    Epaisseur = float(Compteur) / 100;
    if (Derniere_Epaisseur != Epaisseur) {
      lcd.setCursor(3, 0); lcd.print("Epaisseur:");
      lcd.setCursor(0, 1); lcd.print("<");
      lcd.setCursor(5, 1); lcd.print(Epaisseur); lcd.print(" mm ");
      lcd.setCursor(15, 1); lcd.print(">");

      Serial.print("Choix de l'epaisseur : "); Serial.print(Epaisseur); Serial.println("mm");
    } Derniere_Epaisseur = Epaisseur;
  }
  Serial.print("Epaisseur Validée : "); Serial.print(Epaisseur); Serial.print("mm || ");
  Epaisseur /= 1000;
  Serial.print(Epaisseur, 5); Serial.println("m \n");
  delay(300);

  //Gestion du mode de mesure
  lcd.clear();
  Compteur, Menu = 1 ;
  while (digitalRead(Bouton) == HIGH) {
    lcd.setCursor(1, 0); lcd.print("Temps Mesure:");
    if (Compteur != 0) {
      if (Compteur > 0) {
        Menu++;
      } else {
        Menu--;
      }

      if (Menu > 3) {
        Menu = 1;
      } if (Menu < 1) {
        Menu = 3;
      }

      switch (Menu) {
        case 1:
          lcd.setCursor(3, 1); lcd.print("< 5  Min > ");
          Temps_Mesure = 300;
          Serial.print("Choix du mode : Mode 1 < "); Serial.print(Temps_Mesure / 60); Serial.println("  min >");
          break;
        case 2:
          lcd.setCursor(3, 1); lcd.print("< 10 Min >");
          Temps_Mesure = 600;
          Serial.print("Choix du mode : Mode 2 < "); Serial.print(Temps_Mesure / 60); Serial.println(" min >");
          break;
        case 3:
          lcd.setCursor(3, 1); lcd.print("< 15 Min >");
          Temps_Mesure = 900;
          Serial.print("Choix du mode : Mode 3 < "); Serial.print(Temps_Mesure / 60); Serial.println(" min >");
          break;
      }

      delay(300);
      Compteur = 0;
    }
  }
  Serial.print("Mode "); Serial.print(Menu); Serial.print(" Validé || "); Serial.print(Temps_Mesure); Serial.println(" s de mesure\n");
  delay(300);



  //Pré-chauffage
  lcd.clear();
  Verification = false;
  Temps_Debut = millis();
  Temps = 0;
  Serial.println("Debut du pré-chauffage");
  while (Verification == false) {
    float Verif_Peltier = 0.0;
    float Verif_Resistance = 0.0;
    int Moyenne_Verif = 10;
    lcd.setCursor(2, 0); lcd.print("Prechauffage");

    for (Loop = 0 ; Loop < Moyenne_Verif ; Loop++) {
      Asservissement();
      Verif_Peltier += Temp_Peltier;
      Verif_Resistance += Temp_Resistance;
      lcd.setCursor(1, 1); lcd.print(Temp_Resistance, 1); lcd.print(" C ");

      if (Temp_Peltier > 10) {
        lcd.setCursor(8, 1);
      } else {
        lcd.setCursor(9, 1);
      }
      lcd.print(" "); lcd.print(Temp_Peltier, 1); lcd.print(" C ");
      delay(500);
    }

    Verif_Resistance /= Moyenne_Verif;
    Verif_Peltier /= Moyenne_Verif;

    Serial.print("Stabilisation chaud : "); Serial.print(Verif_Resistance); Serial.print(" °C || Stabilisation froid : "); Serial.print(Verif_Peltier); Serial.println(" °C ");
    Temps = (millis() - Temps_Debut) / 1000;
    Serial.print("Préchauffage : ");
    if (Temps >= Prechauffage) {
      Serial.println("Terminé");
    } else {
      Serial.print(Temps / Prechauffage * 100, 0); Serial.println("%");
    }
    float Marge_Erreur = 0.3;
    if (Verif_Peltier < Consigne_Froid + Marge_Erreur && (Verif_Peltier > Consigne_Froid - Marge_Erreur  ) && (Verif_Resistance < Consigne_Chaud + Marge_Erreur && Verif_Resistance > Consigne_Chaud - Marge_Erreur ) && (Temps > Prechauffage)) {
      Verification = true;
      Serial.println("Fin de la phase de préchauffage");
    }
  } delay(300);


  //Mesure de la resistance thermique
  Serial.println("Demarage de la mesure");
  Compteur = 0 ;
  Menu = 1;
  Energie_Total = 0;
  Loop = 1;
  long Compte_Tours = 50000;
  Temps_Debut = millis();
  Temps = 0;
  MsTimer2::start();
  Timer = false;
  while (Temps <= Temps_Mesure) {
    if (Compte_Tours <= 50000) {
      if (Timer == true) {
        Timer = false;
        Puissance = ina219.getPower_mW() / 1000;
        Energie = Puissance * Intervalle_Mesure / 1000;
        Energie_Total += Energie;
      }
      if (Compte_Tours % 10000 == 1) {
        Asservissement();
      }
    } else {
      Compte_Tours = 0;

      lcd.setCursor(2, 0); lcd.print("Mesure : "); lcd.print(Temps / Temps_Mesure * 100, 0); lcd.print("%");
      if (Compteur != 0) {
        if (Compteur > 0) {
          Menu++;
        } else {
          Menu--;
        }

        if (Menu > 5) {
          Menu = 1;
        } if (Menu < 1) {
          Menu = 5;
        }
      }

      lcd.setCursor(0, 1); lcd.print("                ");
      switch (Menu) {

        case 1:
          lcd.setCursor(1, 1); lcd.print(Temp_Resistance, 1); lcd.print(" C ");
          if (Temp_Peltier > 10) {
            lcd.setCursor(8, 1);
          } else {
            lcd.setCursor(9, 1);
          }
          lcd.print(" "); lcd.print(Temp_Peltier, 1); lcd.print(" C ");
          lcd.setCursor(7, 1); lcd.print("||");
          break;

        case 2:
          lcd.setCursor(0, 1); lcd.print("Energie :"); lcd.print(Energie_Total); lcd.print("J");
          break;

        case 3:
          lcd.setCursor(0, 1); lcd.print("Puissance :"); lcd.print(Puissance_Moyenne); lcd.print("W");
          break;

        case 4:
          lcd.setCursor(1, 1); lcd.print("Lambda : "); lcd.print(Lambda,3);
          break;

        case 5:
          lcd.setCursor(0, 1); lcd.print("Resist_th :"); lcd.print(Resistance_th,3);
          break;
      } Compteur = 0;

      Moyenne_Froid += Temp_Peltier;  Moyenne_Chaud += Temp_Resistance;
      Detla_Temp = (Moyenne_Chaud - Moyenne_Froid) / Loop ;
      Puissance_Moyenne = Energie_Total / Temps;
      Lambda = (Puissance_Moyenne * Epaisseur) / (Surface * Detla_Temp);
      Resistance_th = Epaisseur / Lambda;

      Serial.print("Avancement mesure : "); Serial.print(Temps / Temps_Mesure * 100); Serial.println(" %");
      Serial.print("\t-Energie consommée : "); Serial.print(Energie_Total); Serial.println(" J");
      Serial.print("\t-Puissance Moyenne : "); Serial.print(Puissance_Moyenne); Serial.println(" W");
      Serial.print("\t-Difference temperature : "); Serial.print(Detla_Temp); Serial.println(" °C");
      Serial.print("\t-Valeur du Lambda : "); Serial.print(Lambda,3); Serial.println(" W.m−1.K−1");
      Serial.print("\t-Valeur de la Resistance Thermique : "); Serial.print(Resistance_th,3); Serial.println(" m2.K.W-1\n");

      Loop++;
      Temps = (millis() - Temps_Debut) / 1000;
    } Compte_Tours++;
  }
  Serial.println("\n\n-------");
  Serial.println("\t\tFin de la mesure\n");
  Serial.print("Avancement mesure : "); Serial.print(Temps / Temps_Mesure * 100); Serial.println(" %");
  Serial.print("Epaisseur : "); Serial.print(Epaisseur * 1000); Serial.println(" mm");
  Serial.print("Energie consommée : "); Serial.print(Energie_Total); Serial.println(" J");
  Serial.print("Puissance Moyenne : "); Serial.print(Puissance_Moyenne); Serial.println(" W");
  Serial.print("Difference temperature : "); Serial.print(Detla_Temp); Serial.println(" °C");
  Serial.print("Valeur du Lambda : "); Serial.print(Lambda,3); Serial.println(" W.m−1.K−1");
  Serial.print("Valeur de la Resistance Thermique : "); Serial.print(Resistance_th,3); Serial.println(" m2.K.W-1");
  Serial.println("-------\n");


  MsTimer2::stop();
  analogWrite(Chauffage, 0);
  analogWrite(Peltier, 0);
  delay(300);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Fin de la mesure");
  lcd.setCursor(3, 1); lcd.print("Resultat :");

  delay(3000);

  //Affichage des résultats
  Compteur = 1;
  Menu = 3 ;
  Serial.print("Compteur : ");
  Serial.println(Compteur);
  while (digitalRead(Bouton) == HIGH) {
    if (Compteur != 0) {
      lcd.clear();
      if (Compteur > 0) {
        Menu++;
      } else {
        Menu--;
      }

      if (Menu > 3) {
        Menu = 1;
      } if (Menu < 1) {
        Menu = 3;
      }

      switch (Menu) {
        case 1:
          lcd.setCursor(0, 0); lcd.print("Epaisseur:"); lcd.print(Epaisseur * 1000); lcd.print("mm");
          lcd.setCursor(0, 1); lcd.print("Puissance:"); lcd.print(Puissance_Moyenne); lcd.print("W");
          break;
        case 2:
          lcd.setCursor(0, 0); lcd.print("DeltaT : "); lcd.print(Detla_Temp);
          lcd.setCursor(0, 1); lcd.print("Flux   : "); lcd.print(Lambda,3);
          break;
        case 3:
          lcd.setCursor(0, 0); lcd.print("Resist_th: "); lcd.print(Resistance_th,3);

          if (Resistance_th > 0.5) {
            lcd.setCursor(4, 1); lcd.print("-Bonne-");
          } else if (Resistance_th < 0.01) {
            lcd.setCursor(3, 1); lcd.print("-Mauvaise-");
          } else {
            lcd.setCursor(3, 1); lcd.print("-Moyenne-");
          }

          break;
      }

      delay(300);
      Compteur = 0;
    }

  }  delay(300);
}


//Fonction initialisant une nouvelle mesure
void Initialisation() {
  Serial.println("\nInitialisation...\n");
  if (Thermometre(3) != 0 ) {
    Temp_Ambiant = Thermometre(3);
  } else {
    Serial.println("Attention themistance ambiante non detectée, veleur par defaut utilisée.");
  }
  Serial.print("Temperature ambiante : "); Serial.print(Temp_Ambiant); Serial.println(" °C");
  Consigne_Chaud = Temp_Ambiant + 1;

  Compteur = 300;
  Epaisseur = Compteur / 100;
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Nouvelle Mesure");
  delay(300);
  for (Loop = 0; Loop <= 8; Loop ++) {
    lcd.setCursor(8 + Loop, 1); lcd.print(".");
    lcd.setCursor(8 - Loop, 1); lcd.print(".");
    delay(64 - Loop * 8);
  } delay(800); lcd.clear();

  Serial.println("\nSytsteme initialisé\n");
}

//Fonction d'aiguillage Thermistance
float Thermometre(int Demande) { // 1:Peltier 2:Resistance 3:Ambiant
  float Temperature = 0.0;
  if (Demande == 1) {
    Temperature = Capteur_temperature(Thermi_Froid);
    //Serial.print("Temperature peltier : "); Serial.println(Temperature);
    return Temperature;
  }
  else if (Demande == 2) {
    Temperature = Capteur_temperature(Thermi_Chaud);
    //Serial.print("Temperature résistance : "); Serial.println(Temperature);
    return Temperature;
  }
  else if (Demande == 3) {
    Temperature = Capteur_temperature(Thermi_Ambiant);
    if (Temperature < 2) {
      Temperature = 0;
    }
    return Temperature;
  }
  else {
    Serial.println("Erreur aiguillage temperture impossible");
    return Temperature;
  }
}

//Fonction Thermometre
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
  if (MoyenneTemperature > Temp_Ambiant + 20) {
    Protection_Temp();
  }
  return MoyenneTemperature ;
}

//Fonction d'interruption encodeur
void ILS_Rotation() {
  Etat_DT = digitalRead(DT);
  if (Etat_DT != DerniereEtat_DT) {
    if (digitalRead(CLK) != Etat_DT) {
      Compteur ++;
    } else {
      Compteur --;
    }
    //Serial.print("Position: "); Serial.println(Compteur);
  }
  DerniereEtat_DT = Etat_DT;
}

//Fonction de protection haute temperature
void Protection_Temp() {
  Serial.println("Surchauffe des composants !");
  Serial.println("Arret du systeme");
  Serial.println("Redemarrage manuel du systeme requis");
  analogWrite(Chauffage, 0);
  analogWrite(Peltier, 0);
  while (true) {
    lcd.clear();
    lcd.setCursor(3, 0), lcd.print("Protection");
    lcd.setCursor(3, 1), lcd.print("Temperture");
    delay(4000);
    lcd.clear();
    lcd.setCursor(2, 0), lcd.print("Redemarrage");
    lcd.setCursor(1, 1), lcd.print("manuel requis");
    delay(4000);
  }
}

//Fonction Asservissement
float PID_errorC, Previous_errorC = 0;
float PID_errorF, Previous_errorF = 0;
int PID_valueC = 0;
int PID_valueF = 0;

int kpC = 62 ;   float kiC = 0.3;   float kdC = 1.2;   //Constante PID Resistance
int kpF = 235;   float kiF = 0.6;   float kdF = 1.2;   //Constante PID Froid

int PID_pC, PID_iC, PID_dC = 0;
int PID_pF, PID_iF, PID_dF = 0;

void Asservissement() {
  Temp_Peltier = Thermometre(1) ;
  Temp_Resistance = Thermometre(2) ;

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

  /*
    Serial.println("--");
    Serial.print("Valeur PID Peltier : "); Serial.print(PID_valueF);
    Serial.print("Valeur PID Resistance : "); Serial.print(PID_valueC);
    Serial.println("--");
  */

  analogWrite(Chauffage, PID_valueC);
  analogWrite(Peltier, PID_valueF);
}

//Fonction d'interruption de puissance
void FCT_Timer() {
  Timer = true;
}
