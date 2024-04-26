#include <WiFi.h>

// Pin definitions
// const int waterLevelPin = 26; <- got rid of water level sensor in the
// prototype.
const int SOIL_MOISTURE_PIN = 27;
const int PUMP_CONTROL_PIN = 14;

// Constants
const int TO_SECONDS = 1000;               // 1000 milliseconds = 1 second
const int TO_MINUTES = 60000;              // 60000 milliseconds = 1 minute
const int PUMP_COOLDOWN = 5 * TO_MINUTES;  // 5 minutes
const int PUMP_DURATION = 10 * TO_SECONDS; // 10 seconds
const int DELTA_TIME = 5 * TO_SECONDS;     // 5 seconds

// Variables
int moistureLevel = 0;      // 0-4096 (In reality 2300 - 3400)
int wateringThreshold = 0;  // Fetched from server
unsigned long lastPump = 0; // Last time the pump was used

// Wifi & webserver information
const char SSID[] = "MarkonNetti";             // <- Your own string here
const char PASSWORD[] = "12345678";            // <- Your own string here
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
  Serial.begin(115200);

  // Comment out the following lines if you dont't want to wait for serial
  /*
while (!Serial) {
  ; // wait for serial port to connect. Needed for native USB port only
}
*/

  // Looks like junk but just don't touch it
  Serial.println();
  Serial.println("    _____ __________  __      __  _________");
  Serial.println("   /  _  \\\\______   \\/  \\    /  \\/   _____/");
  Serial.println("  /  /_\\  \\|     ___/\\   \\/\\/   /\\_____  \\ ");
  Serial.println(" /    |    \\    |     \\        / /        \\");
  Serial.println(" \\____|__  /____|      \\__/\\  / /_______  /");
  Serial.println("         \\/                 \\/          \\/ ");
  Serial.println("\n > Automatic plant watering system by PDF group 20 \n");

  Serial.println(" > Starting setup");

  // Allow values up to 4096 for analogRead
  analogReadResolution(12);
  pinMode(PUMP_CONTROL_PIN, OUTPUT);
  digitalWrite(PUMP_CONTROL_PIN, LOW);

  // Set the last pump time to current time minus the cooldown time
  lastPump = millis() - PUMP_COOLDOWN;

  // Connect to wifi
  connect_wifi();

  Serial.println(" > Beginning main loop execution \n");
  Serial.println(" ============================= \n");
}

// Wifi connection function. Connects to the wifi network or "hangs" if the wifi
// shield is not present.
void connect_wifi() {
  // Check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(" >>> WiFi shield not present");
    // don't continue:
    while (true)
      ;
  }

  // Keep trying to connect to the WiFi network
  while (status != WL_CONNECTED) {
    // Print connection information
    Serial.print(" > Attempting to connect to WPA SSID: ");
    Serial.println(SSID);

    status = WiFi.begin(SSID, PASSWORD); // Connect to WPA/WPA2 network
    delay(10 * TO_SECONDS); // Delay to allow connection to be established
  }

  // Print on successful connection
  Serial.print(" >> WiFi Connection Established!! SSID: ");

  // Print the SSID and IP address in the connected network
  Serial.print(WiFi.SSID());
  Serial.print(", ");
  Serial.print("Assinged IP in network: ");
  Serial.print(WiFi.localIP());
  Serial.println("\n");
}

// Main functions (loop and utility functions)
// Main execution loop
void loop() {
  // Execute the main loop every DELTA_TIME
  delay(DELTA_TIME);
  // Read the moisture level
  moistureLevel = readMoistureLevel();

  // Print the moisture level
  Serial.println(" > Moisture level: " + String(moistureLevel));

  // Update the moisture level to the server
  Response res = post("/set-moisture", String(moistureLevel));
  if (res.status != 200) {
    Serial.println(" >>> Failed to update moisture level");
  }

  // Fetch the watering threshold from the server
  res = get("/trigger");
  if (res.status == 200) {
    if (wateringThreshold != res.body.toInt()) {
      wateringThreshold = res.body.toInt();
      Serial.println(" > Watering threshold updated: " +
                     String(wateringThreshold));
    }
  } else {
    Serial.println(" > Failed to fetch threshold");
  }

  // If the watering threshold is 0, don't water the plant
  if (wateringThreshold == 0) {
    return;
  }

  // Allow pump to be used only once every 5 minutes
  if (timeSince(lastPump) > PUMP_COOLDOWN || wateringThreshold == 42069) {
    // Moisture percentage can be calculated but it might not be practical, nor
    // an accurate representation of the moisture level.
    // int moisturePercent = map(moistureLevel, 2300, 3400, 0, 100);

    // If the moisture level is below the threshold, water the plant
    if (moistureLevel < wateringThreshold) {
      lastPump = millis();
      pumpWater(PUMP_DURATION);
    }
  }

  Serial.println("\n ============================= \n");
}
// Function to read the moisture level from the soil
int readMoistureLevel() { return analogRead(SOIL_MOISTURE_PIN); }

// Returns the time since the given time in milliseconds for easier readability
// of if statements
int timeSince(unsigned long time) { return millis() - time; }

// Function to pump water for a given duration
void pumpWater(int duration) {
  Serial.println(" > Pumping water for " + String(duration / TO_SECONDS) +
                 " seconds");
  digitalWrite(PUMP_CONTROL_PIN, HIGH); // Turn on pump
  delay(duration);
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

// Function to send a GET request to the server. Returns the response as a
// struct. The struct contains the necessary information to handle the response
// and a little extra for debugging
Response get(String path) {
  if (client.connect(HOST_NAME, 80)) { // Try to connect to the server
    // Make a HTTP request:
    client.println("GET " + path + " HTTP/1.1");
    client.println("Host: " + HOST_NAME_S);
    client.println("MarkoAuth: Markonsalasana2024");
    client.println("Connection: close");
    client.println();
  } else {
    // Cannot connect to the server
    Serial.println(" >>> Cannot connect to server");
    return Response{0, "", ""};
  }

  // wait for data until timeout
  unsigned long started = millis();
  while (!client.available()) {
    delay(1); // MS delay to keep timeout accurate

    if (timeSince(started) > HTTP_TIMEOUT) {
      Serial.println(" >>> Timeout");
      client.stop();
      return Response{0, "", ""};
    }
  }
  return parseResponse();
}

// Function to send a POST request to the server. Returns the response as a
// struct. The struct contains the necessary information to handle the response
// and a little extra for debugging
Response post(String path, String body) {
  if (client.connect(HOST_NAME, 80)) { // Try to connect to the server
    // Make a HTTP request:
    client.println("POST " + path + " HTTP/1.1");
    client.println("Host: " + HOST_NAME_S);
    client.println("MarkoAuth: Markonsalasana2024");
    client.println("Connection: close");
    client.println("Content-Type: text/plain");
    client.println("Content-Length: " + String(body.length()));
    client.println();
    client.println(body);
  } else {
    // Cannot connect to the server
    Serial.println(" >>> Cannot connect to server");
    return Response{0, "", ""};
  }

  // wait for data until timeout
  unsigned long started = millis();
  while (!client.available()) {
    delay(1); // MS delay to keep timeout accurate

    if (timeSince(started) > HTTP_TIMEOUT) {
      Serial.println(" >>> Timeout");
      client.stop();
      return Response{0, "", ""};
    }
  }
  return parseResponse();
}

// Function to parse HTTP/1.1 response into status code, raw response and body
// text. Lot of different condition handling are missing, but it should work for
// this.
Response parseResponse() {
  String rawResponse = ""; // Raw response string (all reads are appended here)

  // Read the first 8 characters of the response (HTTP/1.1)
  for (int i = 0; i < 8; i++) {
    char c = client.read();
    rawResponse += c;
  }

  // Make sure the response is HTTP/1.1
  if (rawResponse != "HTTP/1.1") {
    Serial.println(" >>> Unsupported HTTP protocol");
    return Response{0, rawResponse, ""};
  }

  // Read the space after HTTP/1.1
  char c = client.read();
  rawResponse += c;

  // Read the status code (3 characters)
  String status = "";
  for (int i = 0; i < 3; i++) {
    char c = client.read();
    rawResponse += c;
    status += c;
  }
  int statusInt = status.toInt(); // Convert status code to integer

  // Read until the end of the response headers
  const String endOfHeaders = "\r\n\r\n";
  while (!rawResponse.endsWith(endOfHeaders) && client.available()) {
    char c = client.read();
    rawResponse += c;
  }

  // Read the body of the response
  String body = "";
  while (client.available()) {
    char c = client.read();
    body += c;
    rawResponse += c;
  }

  return Response{statusInt, rawResponse, body};
}

/**
int readWaterLevel() {
  int val = analogRead(waterLevelPin);
  return val;
}  <- got rid of water level sensor in the prototype.
**/
