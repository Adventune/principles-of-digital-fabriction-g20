#include <WiFi.h>

//const int waterLevelPin = 26; <- got rid of water level sensor in the prototype.
const int soilMoisturePin = 27;
const int pumpControlPin = 14;

const int cooldown = 300000;
const int deltaTime = 5000;

int pumpCooldown = 0;
int waterLevel = 0;
int moistureLevel = 0;
int pumpForXseconds = 5;

//Wifi information:
const char* ssid = "ssid"; // <- Your own string here
const char* password = "pass"; // <- Your own string here

int thresHold = 0;
int fetchTimer = 0;
int fetchCooldown = 10000;

int checkThresholdTimer = 0;
int checkThresholdCooldown = 600000;

//TODO: parameters for making http requests to the server?

bool wifi_connected() {
  return WiFi.status() == WL_CONNECTED;
}

void connect_wifi(const char *ssid, const char *pass) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting...");
  while(!wifi_connected()) {
    delay(500);
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi Connection Established!!");
  Serial.println(WiFi.SSID());

  Serial.print("Assinged IP: ");
  Serial.println(WiFi.localIP());

}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  delay(1000);
  pinMode(pumpControlPin, OUTPUT);
  digitalWrite(pumpControlPin, LOW);

  //Connect to wifi
  connect_wifi(ssid, password);
  //TODO: Connect to server?

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
    pumpWater(pumpForXseconds);
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

/**int readWaterLevel() {
  int val = analogRead(waterLevelPin);
  return val;
}  <- got rid of water level sensor in the prototype.  **/
