//LCD files
#include <LiquidCrystal_I2C.h>

//nodemcu file
#include <ESP8266WiFi.h>

//Firebase File
#include <FirebaseArduino.h>

//RFID reader Files
#include <SPI.h>
#include <MFRC522.h>

//Firebase Variables
//#define FIREBASE_HOST "the-wah.firebaseio.com"
//#define FIREBASE_AUTH "YzhBXfwbi601Ot99znqhrpStTLNYm5D1UDSSoSEO"
#define FIREBASE_HOST "thewah-e768a.firebaseio.com"
#define FIREBASE_AUTH "qRmQP97rP51t3xU6qrBhTLF1SaA9Q9ReFBhxmF74"
#define DB_PATH "Assistants/a45vuSucZIaD6BSx49J7HEJJbu12/Doctors/-L6f2Oi2eHAls0g6JwGl/Tokens/"
#define Dr_STATE "Assistants/a45vuSucZIaD6BSx49J7HEJJbu12/Doctors/-L6f2Oi2eHAls0g6JwGl/state"
#define bookedTokensPath "Assistants/a45vuSucZIaD6BSx49J7HEJJbu12/Doctors/-L6f2Oi2eHAls0g6JwGl/bookedTokens"

//Wifi Variables
#define WIFI_SSID "beyond"
#define WIFI_PASSWORD "royalmen"

//nodemcu pins
#define RST_PIN         0           
#define SS_PIN          2         

//required variables
LiquidCrystal_I2C lcd(0x27,16,2);
int currentToken = -1;
String  LCD_line1 = "Dr. P. Kumar", LCD_line2 = "NEXT : 1";
MFRC522 mfrc522(SS_PIN, RST_PIN); 

//userdefined functions

void setup() {
//init SPI, Firebase, LCD and MFRC522                                          
  SPI.begin();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  mfrc522.PCD_Init();
  lcd.init();
  lcd.backlight();

//connecting..
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print(F("Connecting.."));


//wifi connect
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  }

//make Dr. Available
  while(true){
    Firebase.setString(Dr_STATE, "Available");
    if(Firebase.success()) break;
  }
  
//Starting Message
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print(F("Dr. P. Kumar"));
  lcd.setCursor(4,1);
  lcd.print(F("Next : 1"));
}

void loop() {
  String  LCD_line1 = "CURRENT : ", LCD_line2 = "NEXT : ";
  int tokenNumber;
  char tokenChar[18];
  String tokenString;
  
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  byte len=18;
  
  MFRC522::StatusCode statuss;

//dectecting new card
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); 

  byte bufer[18];

//authenticating
  statuss = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid));
  if (statuss != MFRC522::STATUS_OK) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

//reading card
  statuss = mfrc522.MIFARE_Read(1, bufer, &len);
  if (statuss != MFRC522::STATUS_OK) {
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

  bool nextTokenIsBooked = false, tokenIsBooked = false;
  int nextToken = tokenNumber;

//get the bookedTokens Array
      FirebaseObject bookedFirebaseObject = Firebase.get(bookedTokensPath);
      if(Firebase.success()){
        JsonArray& bookedTokens = (bookedFirebaseObject.getJsonVariant()).as<JsonArray>();

//check if tokenIsBooked
            for(auto value : bookedTokens){
              if(value.as<int>() == tokenNumber){
                tokenIsBooked = true;
                break;
              }
            }

  if(!tokenIsBooked){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Token Not Booked");  
        lcd.setCursor(2,1);
        lcd.print("Return it!!");
        
        delay(2000);
        
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print(::LCD_line1);
        lcd.setCursor(1,1);
        lcd.print(::LCD_line2); 

        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
  }
  
// get next Token
          while(!nextTokenIsBooked && nextToken <= 100){
            nextToken++;
            for(auto value : bookedTokens){
              if(value.as<int>() == nextToken){
                nextTokenIsBooked = true;
                break;
              }
            }
          }        
      }
      else{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("RETAP Your Card");
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
      }
 
//LCD variables UPDATE
  LCD_line1 = LCD_line1 + String(tokenNumber);
  ::LCD_line1 = LCD_line1;
  if(nextTokenIsBooked && nextToken<101){
  LCD_line2 = LCD_line2 + String(nextToken);
//LCD print
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print(LCD_line1);
    lcd.setCursor(4,1);
    lcd.print(LCD_line2);   
  }
  else{
  LCD_line2 = "No more Tokens";
//LCD print
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print(LCD_line1);
    lcd.setCursor(1,1);
    lcd.print(LCD_line2);  
  } 
  ::LCD_line2 = LCD_line2;
  
//update to Firebase---------------------------------------------
  String dbPath;
  const String state = "/state";

//updating used
  if(currentToken>0){
    dbPath = DB_PATH + String(currentToken) + state;
    Firebase.setString(dbPath, "used");
    if (Firebase.failed()) {  
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
    }  
  }

//updating current
  dbPath = DB_PATH + String(tokenNumber) + state;  
  Firebase.setString(dbPath, "current");
  if (Firebase.failed()) {  
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      return;
  }
  currentToken = tokenNumber;
  
//log upload to firebase------------------------------------------
//  Firebase.pushInt("logs", tokenNumber);
//  if (Firebase.failed()) {   
//      mfrc522.PICC_HaltA();
//      mfrc522.PCD_StopCrypto1();
//      return;
//  }

  delay(1000); 
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
