#include <Arduino.h>
#include <string>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include "DHT.h"
#include <ESP32Servo.h>
#include <Adafruit_LiquidCrystal.h>
#include <WebSocketsServer.h>
/**********************************/
Servo myservo;

Adafruit_LiquidCrystal lcd_1(0);
/*****************************/
#define echoPin1 13 
#define trigPin1 12
#define echoPin2 14 
#define trigPin2 27
#define echoPin3 26 
#define trigPin3 25
#define servopin 17
/******************/
int cardecdis = 3;
int cardecdismin = 1;
int cardecdismax = 7;
/**********************/
int parkstate1;
int parkstate2;
int parkstate3;
int parkstate4;
int parkstate5;
int parkstate6;
/**********************/
int led1r = 2;
int led1g = 4;
int led2r = 17;
int led2g = 16;
int led3r = 18;
int led3g = 19;

/*********************/
long duration1;
int distance1;
long duration2;
int distance2;
long duration3;
int distance3;
/******************************/
/**********************************/
void ultra1() {
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = duration1 * 0.034 / 2;
  delay(200);
  Serial.print("Distance1 ");
  Serial.print(distance1);
  Serial.println(" cm");
}

/*****************************/

void ultra2() {
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  distance2 = duration2 * 0.034 / 2;
  delay(200);
  Serial.print("Distance2 ");
  Serial.print(distance2);
  Serial.println(" cm");
}

/**********************************/
               
void ultra3() {
  digitalWrite(trigPin3, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin3, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin3, LOW);
  duration3 = pulseIn(echoPin3, HIGH);
  distance3 = duration3 * 0.034 / 2;
  delay(200);
  Serial.print("Distance3 ");
  Serial.print(distance3);
  Serial.println(" cm");
}

/*****************************/
/**********************************/

void cardec() {
  if (distance1 < cardecdis && distance1 > cardecdismin && distance1 < cardecdismax) {
    parkstate1 = 1;
    lcd_1.setCursor(1, 1);
    lcd_1.print("c|");
    analogWrite(led1g,100);
  } else if (distance1 < cardecdismin || distance1 > cardecdismax){
    parkstate1 = 2;
    lcd_1.setCursor(1, 1);
    lcd_1.print("T|");
  } else {
    parkstate1 = 3;
    lcd_1.setCursor(1, 1);
    lcd_1.print("n|");
  }

  if (distance2 < cardecdis && distance2 > cardecdismin && distance2 < cardecdismax) {
    parkstate2 = 1;
    lcd_1.setCursor(3, 1);
    lcd_1.print("c|");
    analogWrite(led2g,100);
  } else if (distance2 < cardecdismin || distance2 > cardecdismax){
    parkstate2 = 2;
    lcd_1.setCursor(3, 1);
    lcd_1.print("T|");
  } else {
    parkstate2 = 3;
    lcd_1.setCursor(3, 1);
    lcd_1.print("n|");
  }

  if (distance3 < cardecdis && distance3 > cardecdismin && distance3 < cardecdismax) {
    parkstate3 = 1;
    lcd_1.setCursor(5, 1);
    lcd_1.print("c|");
  } else if (distance3 < cardecdismin || distance3 > cardecdismax){
    parkstate3 = 2;
    lcd_1.setCursor(5, 1);
    lcd_1.print("T|");
  } else {
    parkstate3 = 3;
    lcd_1.setCursor(5, 1);
    lcd_1.print("n|");
  }
}

/****************************/
// Replace with your network credentials
const char* ssid = "Jones";
const char* password = "BloodofJesus";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket ws("/ws");
bool motorRunning = false;

// Define LED pins for each parking spot
const int numSpots = 6;
const int ledPins[numSpots] = {2, 4, 16, 17, 18, 19}; // Adjust the pin numbers according to your setup

String message = "";
String sliderValue1 = "0";

int dutyCycle1;

// Setting PWM properties
const int freq = 5000;
const int ledChannel1 = 0;
const int resolution = 8;

// Json Variable to Hold Slider Values
JSONVar sliderValues;

// Get Slider Values
String getSliderValues() {
  sliderValues["sliderValue1"] = String(sliderValue1);
  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

// Initialize SPIFFS
void initFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  } else {
    Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void notifyClients(String sliderValues) {
  ws.textAll(sliderValues);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data; // Convert char* to String
    Serial.println(message);

    for (int spot = 1; spot <= numSpots; spot++) {
      String confirmationMessage = "confirmReservation " + String(spot) + " reserved";
      String cancellationMessage = "cancelReservation " + String(spot) + " reserved";

      if (message.indexOf(confirmationMessage) != -1) {
        digitalWrite(ledPins[spot - 1], HIGH);
        Serial.println("LED for spot " + String(spot) + " turned ON.");
      }
      
      if (message.indexOf(cancellationMessage) != -1) {
        digitalWrite(ledPins[spot - 1], LOW);
        Serial.println("LED for spot " + String(spot) + " turned OFF.");
      }
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  lcd_1.begin(16, 2);
  lcd_1.print("Parking Space");

  myservo.attach(servopin);

  for (int i = 0; i < numSpots; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  initFS();
  initWiFi();

  ledcSetup(ledChannel1, freq, resolution);

  // Attach the channel to the GPIO to be controlled
  initWebSocket();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");

  server.begin();
}


void loop() {
  //readSensorData();
  ledcWrite(ledChannel1, dutyCycle1);
  ws.cleanupClients();
  ultra1();
  delay(20);
  ultra2();
  delay(20);
  ultra3();
  delay(20);
  cardec();
  delay(3000);
}
