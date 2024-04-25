#include <WiFi.h>

// Pin definitions
// const int waterLevelPin = 26; <- got rid of water level sensor in the
// prototype.
const int SOIL_MOISTURE_PIN = 27;
const int PUMP_CONTROL_PIN = 14;

// Constants
const int TO_SECONDS = 1000;              // 1000 milliseconds = 1 second
const int TO_MINUTES = 60000;             // 60000 milliseconds = 1 minute
const int PUMP_COOLDOWN = 5 * TO_MINUTES; // 5 minutes
const int PUMP_DURATION = 5 * TO_SECONDS; // 5 seconds
const int DELTA_TIME = 5 * TO_SECONDS;    // 5 seconds

// Variables
int moistureLevel = 0;      // 0-4096 (In reality 2300 - 3400)
int wateringThreshold = 0;  // Fetched from server
unsigned long lastPump = 0; // Last time the pump was used

// Wifi & webserver information
const char SSID[] = "";                        // <- Your own string here
const char PASSWORD[] = "";                    // <- Your own string here
const char HOST_NAME[] = "pdf-g20.binop.fi";   // hostname of web server
const String HOST_NAME_S = "pdf-g20.binop.fi"; // hostname of web server
const int HTTP_TIMEOUT = 10000;                // 10 seconds
int status = WL_IDLE_STATUS;                   // the Wifi radio's status
WiFiClient client;

// HTTP response struct
struct Response {
  int status;
  String raw;
  String body;
};

// Setup functions (arduino setup and utility functions)
// Arduino setup function
void setup() {
  // Begin serial communication
  Serial.begin(9600);

  // Comment out the following lines if you dont't want to wait for serial
  while (Serial.available() == 0) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Allow values up to 4096 for analogRead
  analogReadResolution(12);
  pinMode(PUMP_CONTROL_PIN, OUTPUT);
  digitalWrite(PUMP_CONTROL_PIN, LOW);

  // Connect to wifi
  connect_wifi();
}

// Wifi connection function. Connects to the wifi network or "hangs" if the wifi
// shield is not present.
void connect_wifi() {
  // Check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true)
      ;
  }

  // Keep trying to connect to the WiFi network
  while (status != WL_CONNECTED) {
    // Print connection information
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(SSID);

    status = WiFi.begin(SSID, PASSWORD); // Connect to WPA/WPA2 network
    delay(8000); // Delay to allow connection to be established
  }

  // Print on successful connection
  Serial.println("WiFi Connection Established!!");

  // Print the SSID and IP address in the connected network
  Serial.println(WiFi.SSID());
  Serial.print("Assinged IP: ");
  Serial.println(WiFi.localIP());
}

// Main functions (loop and utility functions)
// Main execution loop
void loop() {
  // Execute the main loop every DELTA_TIME

  delay(DELTA_TIME);

  // Read the moisture level
  moistureLevel = readMoistureLevel();

  // If wateringThreshold has not been set, update moisture to server and try to
  // fetch threshold
  if (wateringThreshold == 0) {
    // TODO: POST moisturelevel
    // TODO: GET threshold
    return;
  }

  // Allow pump to be used only once every 5 minutes
  if (timeSince(lastPump) > PUMP_COOLDOWN) {
    // Moisture percentage can be calculated but it might not be practical, nor
    // an accurate representation of the moisture level.
    // int moisturePercent = map(moistureLevel, 2300, 3400, 0, 100);

    // If the moisture level is below the threshold, water the plant
    if (moistureLevel < wateringThreshold) {
      lastPump = millis();
      pumpWater(PUMP_DURATION);
    }
  }
}
// Function to read the moisture level from the soil
int readMoistureLevel() { return analogRead(SOIL_MOISTURE_PIN); }

// Returns the time since the given time in milliseconds for easier readability
// of if statements
int timeSince(unsigned long time) { return millis() - time; }

// Function to pump water for a given duration
void pumpWater(int duration) {
  digitalWrite(PUMP_CONTROL_PIN, HIGH); // Turn on pump
  delay(1000 * duration);
  digitalWrite(PUMP_CONTROL_PIN, LOW); // Turn off pump
}

// HTTP request functions
/*
In arduino requests are written line by line to the client object.

Format is as follows:
METHOD PATH HTTP_VERSION
Header1: value1
Header2: value2
...
HeaderN: valueN

Body

Example of a GET request:
GET /api/v1/moisture HTTP/1.1
Host: pdf-g20.binop.fi
Connection: close

And in arduino code:
client.println("GET /api/v1/moisture HTTP/1.1");
client.println("Host: pdf-g20.binop.fi");
client.println("Connection: close");

More info on HTTP messages:
https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages
*/

// Function to send a GET request to the server. Returns true if the request was
// successful
bool get(String path) {
  if (client.connect(HOST_NAME, 80)) { // Try to connect to the server
    // Connection successful
    Serial.println("Connected");
    // Make a HTTP request:
    client.println("GET " + path + " HTTP/1.1");
    client.println("Host: " + HOST_NAME_S);
    client.println("Connection: close");
    client.println();
  } else {
    // Cannot connect to the server
    Serial.println("Connection failed");
    return false;
  }

  // wait for data until timeout
  unsigned long started = millis();
  while (!client.available()) {
    delay(1); // MS delay to keep timeout accurate

    if (timeSince(started) > HTTP_TIMEOUT) {
      Serial.println("Timeout");
      client.stop();
      return false;
    }
  }
  return true;
}

// Function to parse HTTP/1.1 response into status code, raw response and body
// text
Response parseResponse() {
  // TODO
  // Read the response from the server
  String rawResponse = "";
  while (client.available()) {
    char c = client.read();
    rawResponse += c;
  }

  // Make sure the response is HTTP/1.1
  if (!client.find("HTTP/1.1")) {
    Serial.println("Unsupported HTTP protocol");
    return Response{0, rawResponse, ""};
  }

  String res = "";
  int st = client.parseInt();
  Serial.println("Status code: " + st);
  return Response{st, rawResponse, res};
}

/**
int readWaterLevel() {
  int val = analogRead(waterLevelPin);
  return val;
}  <- got rid of water level sensor in the prototype.
**/
