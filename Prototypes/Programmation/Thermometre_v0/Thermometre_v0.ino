#define Thermistance A2

void setup() {
  // put your setup code here, to run once:
  pinMode(Thermistance , INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  float Tension = 0;
  float Temperature = 0;
  float Resistance = 0;
  float MoyenneTemperature = 0;
  int Compte_tours = 200;
  
  for (int x = 0 ; x < Compte_tours ; x++)
  {
    Tension = analogRead(Thermistance) * 5.00 / 1023.00 ;
    Resistance = Tension*9.8/(5-Tension);
    Temperature += 39.077 * exp(-(Resistance - 5.08) / 9.802) + 1.294;
  }
  MoyenneTemperature = Temperature / Compte_tours;
  Serial.print(Tension);Serial.print(" V : ");
  Serial.print(Resistance);Serial.print(" KiloOhm : ");
  Serial.print(MoyenneTemperature);Serial.println("°C");
}
