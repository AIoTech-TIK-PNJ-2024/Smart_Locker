#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WebServer.h>

#define SS_PIN 21   // SDA
#define RST_PIN 22  // RST
#define BUZZER_PIN 26   // Buzzer
#define LED_PIN 27      // LED
#define SOLENOID_PIN 5  // Solenoid Door Lock

const char* ssid = "G";
const char* password = "12345678";
WebServer server(80);
 
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key; 
byte nuidPICC[4];

void setup() { 
  Serial.begin(9600);
  SPI.begin(); 
  rfid.PCD_Init();
  pinMode(BUZZER_PIN, OUTPUT); // Buzzer
  pinMode(LED_PIN, OUTPUT); // LED
  pinMode(SOLENOID_PIN, OUTPUT); // Solenoid

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  digitalWrite(SOLENOID_PIN, HIGH);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/unlock", handleUnlock);
  server.begin();
}
 
void loop() {
  server.handleClient();
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    tone(BUZZER_PIN,1000);
    delay(200);
    noTone(BUZZER_PIN);
    delay(200);
    tone(BUZZER_PIN,1000);
    delay(200);
    noTone(BUZZER_PIN);
    return;
  }

  if( rfid.uid.uidByte[0] != nuidPICC[0] || 
      rfid.uid.uidByte[1] != nuidPICC[1] || 
      rfid.uid.uidByte[2] != nuidPICC[2] || 
      rfid.uid.uidByte[3] != nuidPICC[3] )
  {
    Serial.println(F("A new card has been detected."));

    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
   
    tone(BUZZER_PIN,1000);
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    noTone(BUZZER_PIN);
    digitalWrite(SOLENOID_PIN,LOW);
    delay(3000);
    digitalWrite(SOLENOID_PIN,HIGH);
    digitalWrite(LED_PIN, LOW);
    return;

  } 
  
  if( rfid.uid.uidByte[0] == nuidPICC[0] || 
      rfid.uid.uidByte[1] == nuidPICC[1] || 
      rfid.uid.uidByte[2] == nuidPICC[2] || 
      rfid.uid.uidByte[3] == nuidPICC[3] ) 
  {
    Serial.println(F("Card read previously."));

    tone(BUZZER_PIN,1000);
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    noTone(BUZZER_PIN);
    digitalWrite(SOLENOID_PIN,LOW);
    delay(3000);
    digitalWrite(SOLENOID_PIN,HIGH);
    digitalWrite(LED_PIN, LOW);
    return; 
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void handleUnlock() {
  tone(BUZZER_PIN,1000);
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  noTone(BUZZER_PIN);
  digitalWrite(SOLENOID_PIN,LOW);    
  delay(3000);
  digitalWrite(SOLENOID_PIN,HIGH);
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "Locker unlocked");
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}
