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
  digitalWrite(22, HIGH);
}
 
void loop() {
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
    tone(26,1000);
    delay(200);
    noTone(26);
    delay(200);
    tone(26,1000);
    delay(200);
    noTone(26);
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
   
    tone(26,1000);
    delay(200);
    noTone(26);
    digitalWrite(22,LOW);
    delay(3000);
    digitalWrite(22,HIGH);
    return;

  } 
  
  if( rfid.uid.uidByte[0] == nuidPICC[0] || 
      rfid.uid.uidByte[1] == nuidPICC[1] || 
      rfid.uid.uidByte[2] == nuidPICC[2] || 
      rfid.uid.uidByte[3] == nuidPICC[3] ) 
  {
    Serial.println(F("Card read previously."));

    tone(26,1000);
    delay(200);
    noTone(26);
    digitalWrite(22,LOW);
    delay(3000);
    digitalWrite(22,HIGH);
    return; 
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
