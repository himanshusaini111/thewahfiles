#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define FIREBASE_HOST "thewah-e768a.firebaseio.com"
#define FIREBASE_AUTH "qRmQP97rP51t3xU6qrBhTLF1SaA9Q9ReFBhxmF74"
#define WIFI_SSID "beyond"
#define WIFI_PASSWORD "royalmen"

#define DB_PATH "a45vuSucZIaD6BSx49J7HEJJbu12/Doctors/-L6f2Oi2eHAls0g6JwGl/Tokens/"

#define RST_PIN         5           
#define SS_PIN          4         

int currentToken = -1;

MFRC522 mfrc522(SS_PIN, RST_PIN); 

void setup() {
  //init serial, SPI and MFRC522
  Serial.begin(9600);                                           
  SPI.begin();                                                  
  mfrc522.PCD_Init();                                           
  Serial.println(F("Read personal data on a MIFARE PICC:")); 

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
  Serial.print(".");
  delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
 
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  
  int tokenNumber;
  char tokenChar[18];
  String tokenString;
  
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  byte block;
  byte len=18;
  
  MFRC522::StatusCode statuss;

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); 

  byte bufer[18];
  block = 1;

  statuss = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid));
  if (statuss != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(statuss));
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

  statuss = mfrc522.MIFARE_Read(block, bufer, &len);
  if (statuss != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(statuss));
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

  //converting buffer to Int--------------------------------------
  for (uint8_t i = 0; i < 16; i++) {
    tokenChar[i] = bufer[i];
  }
  tokenChar[17] = '\0';
  tokenString = tokenChar;
  tokenNumber = tokenString.toInt();
  Serial.println(tokenNumber);
  //---------------------------------------------------------------   
  //update to Firebase---------------------------------------------
  String dbPath, state = "/state";

  if(currentToken>0){
    dbPath = DB_PATH + currentToken + state;
    Firebase.setString(dbPath, "used");
    if (Firebase.failed()) {
        Serial.println("1 " + Firebase.error());  
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
    }  
  }

  dbPath = DB_PATH + tokenNumber + state;
    
  Firebase.setString(dbPath, "current");
  if (Firebase.failed()) {
      Serial.println("2 " + Firebase.error());  
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      return;
  }

  //----------------------------------------------------------------

  //log upload to firebase------------------------------------------
  Firebase.pushInt("logs", tokenNumber);
  if (Firebase.failed()) {
    
      Serial.println("3 " + Firebase.error());  
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      return;
  }
  //----------------------------------------------------------------

  Serial.println(F("\n**End Reading**\n"));

  delay(1000); 

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
