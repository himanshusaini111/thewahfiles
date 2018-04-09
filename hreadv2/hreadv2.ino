//LCD files
#include <LiquidCrystal_I2C.h>

//nodemcu file
#include <ESP8266WiFi.h>

//Firebase File
#include <FirebaseArduino.h>

//RFID reader Files
#include <SPI.h>
#include <MFRC522.h>

#define FIREBASE_HOST "the-wah.firebaseio.com"
#define FIREBASE_AUTH "q9lchUSmqsCKVzfpT19eRZmbC0UtUZIha5cFhHSO"
#define DB_PATH "Assistants/a45vuSucZIaD6BSx49J7HEJJbu12/Doctors/-L6f2Oi2eHAls0g6JwGl/Tokens/"

#define WIFI_SSID "beyond"
#define WIFI_PASSWORD "royalmen"

#define RST_PIN         0           
#define SS_PIN          2         

//required variables and 
LiquidCrystal_I2C lcd(0x27,16,2);
int currentToken = -1;
MFRC522 mfrc522(SS_PIN, RST_PIN); 

void setup() {
  //init serial, SPI, Firebase, LCD and MFRC522
  Serial.begin(9600);                                           
  SPI.begin();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  mfrc522.PCD_Init();
  lcd.init();

  lcd.backlight();
  Serial.println(F("Read a MIFARE PICC:")); 

  //wifi connect
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
  Serial.print(".");
  delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  
  int tokenNumber;
  char tokenChar[18];
  String tokenString, LCD_line1 = "CURRENT : ", LCD_line2 = "NEXT : ";
  
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

  //LCD variables UPDATE
  LCD_line1 = LCD_line1 + String(tokenNumber);
  LCD_line2 = LCD_line2 + String(tokenNumber + 1);

  //LCD print
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print(LCD_line1);
  lcd.setCursor(4,1);
  lcd.print(LCD_line2);
  
  //update to Firebase---------------------------------------------
  String dbPath, state = "/state";

  if(currentToken>0){
    dbPath = DB_PATH + String(currentToken) + state;
    Firebase.setString(dbPath, "used");
    if (Firebase.failed()) {
        Serial.println("1 " + Firebase.error());  
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
    }  
  }
  
  dbPath = DB_PATH + String(tokenNumber) + state;  
  Firebase.setString(dbPath, "current");
  if (Firebase.failed()) {
      Serial.println("2 " + Firebase.error());  
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      return;
  }
  currentToken = tokenNumber;
  
  //log upload to firebase------------------------------------------
  Firebase.pushInt("logs", tokenNumber);
  if (Firebase.failed()) { 
      Serial.println("3 " + Firebase.error());  
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      return;
  }

  Serial.println(F("\n**End Reading**\n"));

  delay(1000); 

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
