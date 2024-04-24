const int waterLevelPin = 26;
const int soilMoisturePin = 27;
const int pumpControlPin = 14;

const int debugPowerPin = 0;

const int cooldown = 300000;
const int deltaTime = 5000;

int pumpCooldown = 0;
int waterLevel = 0;
int moistureLevel = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  delay(1000);
  pinMode(pumpControlPin, OUTPUT);
  digitalWrite(pumpControlPin, LOW);
  digitalWrite(debugPowerPin, HIGH);
}

void loop() {
  delay(deltaTime);
  if(pumpCooldown > 0){
    pumpCooldown -= deltaTime;
    Serial.print("Pump on cooldown: ");
    Serial.println(pumpCooldown);
  }
  if(pumpCooldown <= 0){
    pumpCooldown = 0;
    controlWatering();
  }

  waterLevel = readWaterLevel();

  moistureLevel = readMoistureLevel();
  Serial.print("Moisture level: ");
  Serial.println(moistureLevel);
  int moisturePercent = map(moistureLevel, 2300, 3400, 0, 100);
  Serial.print("Moisture level percent: ");
  Serial.println(moisturePercent);

  if(waterLevel < 1000){
    Serial.println(waterLevel);
    Serial.println("Water level low.");
  }
  else {
    Serial.println(waterLevel);
    Serial.println("Water level ok.");
  }

}

void controlWatering(){
  moistureLevel = readMoistureLevel();
  int moisturePercent = map(moistureLevel, 2300, 3400, 0, 100);
  if(moisturePercent < 65){
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
