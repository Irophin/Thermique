#include <Adafruit_INA219.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define Thermi_Froid A13
#define Thermi_Chaud A14

#define Relais_Froid 10
#define Relais_Chaud 9

LiquidCrystal_I2C lcd(0x27 , 20, 3);

Adafruit_INA219 ina219;

void setup() {
  pinMode(Thermi_Froid , INPUT);
  pinMode(Thermi_Chaud , INPUT);

  pinMode(Relais_Froid , OUTPUT);
  pinMode(Relais_Chaud , OUTPUT);

  lcd.backlight();
  lcd.init();
  lcd.clear();

  ina219.begin();
  uint32_t currentFrequency;

  Serial.begin(9600);
}

void loop() {
  float Temp_Peltier = Thermometre("Peltier") ;
  float Temp_Resistance = Thermometre("Resistance") ;

  if (Temp_Peltier >= 10) {
    digitalWrite(Relais_Froid, HIGH);
  } else {
    digitalWrite(Relais_Froid, LOW);
  }
  if (Temp_Resistance <= 40) {
    digitalWrite(Relais_Chaud, HIGH);
  } else {
    digitalWrite(Relais_Chaud, LOW);
  }
  float Intensite = abs(ina219.getCurrent_mA() / 1000);

  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Plaque de peltier :");
  lcd.setCursor(0, 1);
  lcd.print(Temp_Peltier, 1); lcd.print(" Degres");

  lcd.setCursor(0, 2);
  lcd.print("Resistance :");
  lcd.setCursor(0, 3);
  lcd.print(Temp_Resistance, 1); lcd.print(" Degres");
  lcd.setCursor(15, 3);
  lcd.print(Intensite,1); lcd.print(" A");
  delay(500);
}

float Thermometre(String Demande) {
  float Temperature = 0;
  if (Demande == "Peltier") {
    Temperature = Capteur_temperature(Thermi_Froid);
    Serial.print("Temperature peltier : "); Serial.println(Temperature);
    return Temperature;

  }
  else if (Demande == "Resistance") {
    Temperature = Capteur_temperature(Thermi_Chaud);
    Serial.print("Temperature résistance : "); Serial.println(Temperature);
    return Temperature;
  }
  else {
    Serial.println("Erreur de la mesure de temperature");
    return Temperature;
  }
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
