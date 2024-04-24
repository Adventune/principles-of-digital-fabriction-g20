const int waterLevelPin = 26;
const int soilMoisturePin = 27;
const int pumpControlPin = 14;

const int debugPowerPin = 0;

const int cooldown = 300000;
const int deltaTime = 5000;

int pumpCooldown = 0;
int waterLevel = 0;
int moistureLevel = 0;

String wifipasswd = "123";
String wifiname = "panoulu";

int thresHold = 0;
int fetchTimer = 0;
int fetchCooldown = 10000;

int checkThresholdTimer = 0;
int checkThresholdCooldown = 600000;

//TODO: server information?

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  delay(1000);
  pinMode(pumpControlPin, OUTPUT);
  digitalWrite(pumpControlPin, LOW);
  digitalWrite(debugPowerPin, HIGH);

  //TODO: Establish wifi connection

  while(thresHold == 0){
    if(fetchTimer <= 0){
      fetchTimer = fetchCooldown;
      int moistureLevel = readMoistureLevel();
      //TODO: POST moisturelevel
      delay(1000); //wait 1 sec before requesting threshold
      //TODO: GET threshold
      //TODO: set threshold
      checkThresholdTimer = checkThresholdCooldown;
    }
    else{
      delay(deltaTime);
      fetchTimer -= deltaTime;
    }
  }
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

  moistureLevel = readMoistureLevel();
  Serial.print("Moisture level: ");
  Serial.println(moistureLevel);

  //TODO: POST moistureLevel
  //TODO: GET

  if(checkThresholdTimer <= 0){
    delay(1000); //wait 1 sec before requesting threshold
      //TODO: GET threshold
      //TODO: if not 0, set threshold
    checkThresholdTimer = checkThresholdCooldown;
  }
  else{
    checkThresholdTimer -= deltaTime;
  }

}

void controlWatering(){
  moistureLevel = readMoistureLevel();
  //int moisturePercent = map(moistureLevel, 2300, 3400, 0, 100);
  if(moistureLevel < thresHold){
    pumpWater(3);
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
