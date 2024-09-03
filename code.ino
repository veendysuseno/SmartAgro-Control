#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "Veendy-Suseno";
const char* password = "12345admin";

// Define pins
#define DHTPIN D2
#define DHTTYPE DHT22
#define SOIL_MOISTURE_PIN A0
#define RELAY_FAN_PIN D5
#define RELAY_PUMP_PIN D6
#define LED_PIN D7

// Initialize the DHT sensor, LCD, and web server
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

// Threshold values
int soilMoistureThreshold = 600;
float tempThreshold = 30.0;
float humidityThreshold = 70.0;

// Variables to store sensor data
float temperature;
float humidity;
int soilMoisture;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize sensors and LCD
  dht.begin();
  lcd.begin();
  lcd.backlight();

  // Initialize pins
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(RELAY_FAN_PIN, OUTPUT);
  pinMode(RELAY_PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Initial states
  digitalWrite(RELAY_FAN_PIN, HIGH); // Fan off
  digitalWrite(RELAY_PUMP_PIN, HIGH); // Pump off
  digitalWrite(LED_PIN, LOW); // LED off

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Define routes
  server.on("/", handleRoot);
  server.on("/toggleFan", handleToggleFan);
  server.on("/togglePump", handleTogglePump);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client requests
  server.handleClient();

  // Read sensor data
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  soilMoisture = analogRead(SOIL_MOISTURE_PIN);

  // Check if any reads failed and exit early
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Display data on LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(humidity);
  lcd.print("%");

  // Control fan based on temperature
  if (temperature > tempThreshold) {
    digitalWrite(RELAY_FAN_PIN, LOW); // Fan on
    digitalWrite(LED_PIN, HIGH); // LED on
  } else {
    digitalWrite(RELAY_FAN_PIN, HIGH); // Fan off
    digitalWrite(LED_PIN, LOW); // LED off
  }

  // Control pump based on soil moisture
  if (soilMoisture > soilMoistureThreshold) {
    digitalWrite(RELAY_PUMP_PIN, LOW); // Pump on
  } else {
    digitalWrite(RELAY_PUMP_PIN, HIGH); // Pump off
  }

  delay(2000); // Delay between readings
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; margin: 0; padding: 0; }";
  html += "h1 { color: #333; }";
  html += ".container { padding: 20px; }";
  html += ".status { font-size: 1.5em; margin: 10px 0; }";
  html += "button { padding: 10px 20px; margin: 10px; font-size: 1em; cursor: pointer; }";
  html += ".fan-button { background-color: #4CAF50; color: white; border: none; }";
  html += ".pump-button { background-color: #008CBA; color: white; border: none; }";
  html += "</style>";
  html += "<script>";
  html += "function toggleFan() { fetch('/toggleFan').then(() => { location.reload(); }); }";
  html += "function togglePump() { fetch('/togglePump').then(() => { location.reload(); }); }";
  html += "</script>";
  html += "</head><body>";
  html += "<div class=\"container\">";
  html += "<h1>Smart Agro System</h1>";
  html += "<div class=\"status\">Temperature: " + String(temperature) + " °C</div>";
  html += "<div class=\"status\">Humidity: " + String(humidity) + " %</div>";
  html += "<div class=\"status\">Soil Moisture: " + String(soilMoisture) + "</div>";
  html += "<button class=\"fan-button\" onclick=\"toggleFan()\">Toggle Fan</button>";
  html += "<button class=\"pump-button\" onclick=\"togglePump()\">Toggle Pump</button>";
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleToggleFan() {
  if (digitalRead(RELAY_FAN_PIN) == HIGH) {
    digitalWrite(RELAY_FAN_PIN, LOW); // Turn on the fan
    digitalWrite(LED_PIN, HIGH); // LED on
  } else {
    digitalWrite(RELAY_FAN_PIN, HIGH); // Turn off the fan
    digitalWrite(LED_PIN, LOW); // LED off
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleTogglePump() {
  if (digitalRead(RELAY_PUMP_PIN) == HIGH) {
    digitalWrite(RELAY_PUMP_PIN, LOW); // Turn on the pump
  } else {
    digitalWrite(RELAY_PUMP_PIN, HIGH); // Turn off the pump
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
