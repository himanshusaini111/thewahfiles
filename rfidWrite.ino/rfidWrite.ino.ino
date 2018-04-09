/*
 * Write personal data of a MIFARE RFID card using a RFID-RC522 reader
 * Uses MFRC522 - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY COOQROBOT. 
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * Hardware required:
 * Arduino
 * PCD (Proximity Coupling Device): NXP MFRC522 Contactless Reader IC
 * PICC (Proximity Integrated Circuit Card): A card or tag using the ISO 14443A interface, eg Mifare or NTAG203.
 * The reader can be found on eBay for around 5 dollars. Search for "mf-rc522" on ebay.com. 
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         0         //D1
#define SS_PIN          2        //D2

MFRC522 mfrc522(SS_PIN, RST_PIN); 

void setup() {
  Serial.begin(9600);        
  SPI.begin();               
  mfrc522.PCD_Init();        
  Serial.println(F("Write personal data on a MIFARE PICC "));
}

void loop() {

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print(F("Card UID:"));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(F(" PICC type: "));   // Dump PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  byte bufer[34];
  byte block;
  MFRC522::StatusCode statuss;
  byte len;

  //Serial.setTimeout(20000L) ;     // wait until 20 seconds for input from serial
  // Ask personal data: Family name
  Serial.println(F("Type Token Number, ending with #"));
  Serial.setTimeout(20000L) ;
  len = Serial.readBytesUntil('#', (char *) bufer, 30) ; // read family name from serial
  for (byte i = len; i < 30; i++) bufer[i] = ' ';     // pad with spaces

  block = 1;
  //Serial.println(F("Authenticating using key A..."));
  statuss = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (statuss != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(statuss));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  statuss = mfrc522.MIFARE_Write(block, bufer, 16);
  if (statuss != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(statuss));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 2;
  //Serial.println(F("Authenticating using key A..."));
  statuss = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (statuss != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(statuss));
    return;
  }

  // Write block
  statuss = mfrc522.MIFARE_Write(block, &bufer[16], 16);
  if (statuss != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(statuss));
    mfrc522.PICC_HaltA(); // Halt PICC
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
    return;
  }
  else Serial.println(F("MIFARE_Write() success: Token Number"));

//  // Ask personal data: First name
//  Serial.println(F("Type First name, ending with #"));
//  len = Serial.readBytesUntil('#', (char *) bufer, 20) ; // read first name from serial
//  for (byte i = len; i < 20; i++) bufer[i] = ' ';     // pad with spaces
//
//  block = 4;
//  //Serial.println(F("Authenticating using key A..."));
//  statuss = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
//  if (statuss != MFRC522::STATUS_OK) {
//    Serial.print(F("PCD_Authenticate() failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(statuss));
//    return;
//  }
//
//  // Write block
//  statuss = mfrc522.MIFARE_Write(block, bufer, 16);
//  if (statuss != MFRC522::STATUS_OK) {
//    Serial.print(F("MIFARE_Write() failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(statuss));
//    return;
//  }
//  else Serial.println(F("MIFARE_Write() success: "));
//
//  block = 5;
//  //Serial.println(F("Authenticating using key A..."));
//  statuss = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
//  if (statuss != MFRC522::STATUS_OK) {
//    Serial.print(F("PCD_Authenticate() failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(statuss));
//    return;
//  }
//
//  // Write block
//  statuss = mfrc522.MIFARE_Write(block, &bufer[16], 16);
//  if (statuss != MFRC522::STATUS_OK) {
//    Serial.print(F("MIFARE_Write() failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(statuss));
//    return;
//  }
//  else Serial.println(F("MIFARE_Write() success: "));


  Serial.println(" ");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD

}
