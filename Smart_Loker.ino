#include <WiFi.h>
#include <ThingsBoard.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <DHT.h>

// WiFi credentials
char ssid[] = "POCO M3 Pro 5G";
char pass[] = "61616161";

// ThingsBoard credentials
#define THINGSBOARD_SERVER "thingsboard.cloud"
#define TOKEN "AohHjPjG1Pnuyny4e2V4"

// DHT11
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ2
#define MQ2PIN 35

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Adjust the address and dimensions as per your LCD module

// Keypad
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};
byte rowPins[ROWS] = { 19, 18, 5, 17 };
byte colPins[COLS] = { 16, 4, 0 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variables
float temperature;
float humidity;
String gasInformation;
String airInformation;
int unit = 0;  // 0: Celsius, 1: Reaumur, 2: Fahrenheit, 3: Kelvin

// Initialize ThingsBoard client
WiFiClient espClient;
ThingsBoard tb(espClient);

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Initialize LCD
  lcd.init();
  lcd.backlight();  // Ensure the backlight is turned on

  // Initialize WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Display initial message
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);
}

void loop() {
  // Reconnect to ThingsBoard if needed
  if (!tb.connected()) {
    // Connect to the ThingsBoard server
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect to ThingsBoard");
      return;
    }
  }

  // Read sensor data
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  int gas = analogRead(MQ2PIN);

  // Check keypad input
  char key = keypad.getKey();
  if (key) {
    switch (key) {
      case '1': unit = 0; break;  // Celsius
      case '2': unit = 1; break;  // Reaumur
      case '3': unit = 2; break;  // Fahrenheit
      case '4': unit = 3; break;  // Kelvin
    }
  }

  // Convert temperature
  float displayTemp = temperature;
  switch (unit) {
    case 1: displayTemp = temperature * 0.8; break;         // Reaumur
    case 2: displayTemp = temperature * 9 / 5 + 32; break;  // Fahrenheit
    case 3: displayTemp = temperature + 273.15; break;      // Kelvin
  }

  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(displayTemp);
  switch (unit) {
    case 0: lcd.print(" C"); break;  // Celsius
    case 1: lcd.print(" R"); break;  // Reaumur
    case 2: lcd.print(" F"); break;  // Fahrenheit
    case 3: lcd.print(" K"); break;  // Kelvin
  }

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");

  lcd.setCursor(0, 2);
  lcd.print("Gas: ");
  if (gas >= 2600) {
    lcd.print("Detected");
    gasInformation = "Detected";
  } else {
    lcd.print("Not Detected");
    gasInformation = "Not Detected";
  }

  lcd.setCursor(0, 3);
  lcd.print("Air Quality: ");
  if (temperature > 36 || humidity > 80 || gas >= 2600) {
    lcd.print("Bad");
    airInformation = "Bad";
  } else {
    lcd.print("Good");
    airInformation = "Good";
  }

  // Update ThingsBoard
  tb.sendTelemetryFloat("humidity", humidity);
  tb.sendTelemetryFloat("temperature", temperature);
  tb.sendTelemetryString("gasInformation", gasInformation.c_str());
  tb.sendTelemetryString("airInformation", airInformation.c_str());

  delay(1000);  // Update interval
}
