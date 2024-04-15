const int waterLevelPin = 26;
const int soilMoisturePin = 27;
const int pumpControlPin = 14;

const int cooldown = 300000;
const int deltaTime = 1000;

int pumpCooldown = 0;
int waterLevel = 0;
int moistureLevel = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  delay(1000);
  pinMode(pumpControlPin, OUTPUT);
  digitalWrite(pumpControlPin, LOW);
}

void loop() {
  delay(deltaTime);
  if(pumpCooldown > 0){
    pumpCooldown -= deltaTime;
  }
  if(pumpCooldown <= 0){
    pumpCooldown = 0;
    controlWatering();
  }

  waterLevel = readWaterLevel();
  Serial.print("Water level: ");
  Serial.println(waterLevel);
  moistureLevel = readMoistureLevel();
  Serial.print("Moisture level: ");
  Serial.println(moistureLevel);
  if(waterLevel < 1000){
    Serial.print("Water level low.");
  }
  else {
    Serial.print("Water level ok.");
  }

}

void controlWatering(){
  moistureLevel = readMoistureLevel();
  if(moistureLevel < 2850){
    pumpWater(2);
    pumpCooldown = cooldown;
  }
}

void pumpWater(int seconds){
  digitalWrite(pumpControlPin, HIGH);
  delay(1000 * seconds);
  digitalWrite(pumpControlPin, LOW);
}

int readMoistureLevel() {
  int val = analogRead(soilMoisturePin);
  return val;
}

int readWaterLevel() {
  int val = analogRead(waterLevelPin);
  return val;
}