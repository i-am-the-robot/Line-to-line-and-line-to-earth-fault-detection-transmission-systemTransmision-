#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <Wire.h>

#include <dummy.h>

const int trigPin = 5;
const int echoPin = 18;
int IR = 32;
int IR2 = 33;
int _moisture,sensor_analog;
const int sensor_pin = A0;  

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
float distanceFeet;

// Replace with your network credentials
const char* ssid = "pearl";
const char* password = "password";

// URL of the cloud server for image prediction
const char* serverUrl = "https://smart-walking-stick.onrender.com/predict";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  pinMode(IR, INPUT); // Declare the sensor as Input.
  pinMode(IR2, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print ESP32 Local IP Address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Route for receiving image via POST request
  server.on("/image", HTTP_POST, [](AsyncWebServerRequest *request){},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) {
        Serial.println("Receiving image...");
        // Open file in write mode if first part of image
        File file = SPIFFS.open("/received_image.jpg", FILE_WRITE);
        if (!file) {
          Serial.println("Failed to open file for writing");
          return;
        }
        file.close();
      }

      // Append image data to file
      File file = SPIFFS.open("/received_image.jpg", FILE_APPEND);
      if (!file) {
        Serial.println("Failed to open file for appending");
        return;
      }
      file.write(data, len);
      file.close();

      if (index + len == total) {
        Serial.println("Image received successfully!");
        request->send(200, "text/plain", "Image received");
        
        // After receiving the image, send it to the cloud server for prediction
        sendImageToServer("/received_image.jpg");
      }
    });

  // Start server
  server.begin();
}

void sendImageToServer(const char* path) {
  HTTPClient http;
  File file = SPIFFS.open(path, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    http.begin(serverUrl);
    http.addHeader("Content-Type", "image/jpeg");

    int fileSize = file.size();
    uint8_t* buffer = new uint8_t[fileSize];
    file.read(buffer, fileSize);
    file.close();

    int httpResponseCode = http.POST(buffer, fileSize);
    delete[] buffer;

    if (httpResponseCode > 0) {
      Serial.print("Response code: ");
      Serial.println(httpResponseCode);

      String response = http.getString();
      Serial.print("Server response: ");
      Serial.println(response);
      // Process the server response here
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void loop() {
// Moisture Sensor
  sensor_analog = analogRead(sensor_pin);
  _moisture = ( 100 - ( (sensor_analog/4095.00) * 100 ) );
  if (_moisture >= 20){
    Serial.println("There is water in front of you!");
    delay(200); 
  }

    //ULTRASONIC SENSOR
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceFeet = distanceCm * 0.0328;
  
  if (distanceFeet <=1) {
    Serial.println("1 feet ahead of you");
    delay(200);
  }
  else if (distanceFeet >1 && distanceFeet <=2){
    Serial.println("2 feet ahead of you");
    delay(200);
  }
  else if (distanceFeet >2 && distanceFeet <=3){
    Serial.println("3 feet ahead of you");
    delay(200);
  }
  else if (distanceFeet >3 && distanceFeet <=4){
    Serial.println("4 feet ahead of you");
    delay(200);
  }
  else if (distanceFeet >4 && distanceFeet <=5){
    Serial.println("5 feet ahead of you");
    delay(200);
  }
  else if (distanceFeet >5 && distanceFeet <=6){
    Serial.println("6 feet ahead of you");
    delay(200);
  }
  else if (distanceFeet >6 && distanceFeet <=7){
    Serial.println("7 feet ahead of you");
    delay(200);
  }
  else {
    Serial.println("Distance unknown");
    delay(200);
  }
  
    //IR sensor
  int s2 = digitalRead(IR2); //This will read the sensor data and store it in a variable. Serial.println(s); //This will print the sensor data to serial monitor.
  int s = digitalRead(IR); //This will read the sensor data and store it in a variable. Serial.println(s); //This will print the sensor data to serial monitor.
  if (s== LOW ){
    Serial.println("TURN LEFT"); 
    delay(200);
  }
 
  else if (s2== LOW){
    Serial.println("TURN RIGHT"); 
    delay(200);
  
  }
  delay(50);
}
