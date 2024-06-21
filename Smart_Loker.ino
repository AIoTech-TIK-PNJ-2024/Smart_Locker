#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5   // SDA
#define RST_PIN 0  // RST
 
MFRC522 rfid(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key; 

byte nuidPICC[4];

void setup() { 
  Serial.begin(9600);
  SPI.begin(); 
  rfid.PCD_Init();
  pinMode(26, OUTPUT); // Buzzer
  pinMode(27, OUTPUT); // LED
  pinMode(22, OUTPUT); // Solenoid

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  digitalWrite(22, HIGH);
}
 
void loop() {
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    tone(26, 1000);
    delay(200);
    noTone(26);
    delay(400);
    tone(26, 1000);
    delay(300);
    noTone(26);
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {

    Serial.println(F("A new card has been detected."));
    tone(26, 1000);
    delay(200);
    noTone(26);
    digitalWrite(22, LOW);
    delay(3000);
    digitalWrite(22, HIGH);

    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  } else if (rfid.uid.uidByte[0] == nuidPICC[0] || 
    rfid.uid.uidByte[1] == nuidPICC[1] || 
    rfid.uid.uidByte[2] == nuidPICC[2] || 
    rfid.uid.uidByte[3] == nuidPICC[3]){
    tone(26, 1000);
    delay(300);
    noTone(26);
    digitalWrite(22, LOW);
    delay(3000);
    digitalWrite(22, HIGH);
    Serial.println(F("Card read previously."));
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
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