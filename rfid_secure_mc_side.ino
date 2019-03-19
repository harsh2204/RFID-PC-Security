/*
 * ------------------------------------
 *             MFRC522      Arduino      
 *             Reader/PCD   Uno/101      
 * Signal      Pin          Pin          
 * ------------------------------------
 * RST/Reset   RST          9            
 * SPI SS      SDA(SS)      10           
 * SPI MOSI    MOSI         11 / ICSP-4  
 * SPI MISO    MISO         12 / ICSP-1  
 * SPI SCK     SCK          13 / ICSP-3  
*/

#include <SPI.h>
#include <MFRC522.h>
#include <MD5.h>
#include <SimpleTimer.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above
#define MAX_LEN         20
#define RED_PIN         7
#define OR_PIN          6
#define GRE_PIN         5

const String authKey1 = ""; //MD5 of your first name
const String authKey2 = ""; //MD5 of your last name

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
SimpleTimer timer;

//*****************************************************************************************//
void setup() {
  Serial.begin(115200);                                           // Initialize serial communications with the 16u2
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
//  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
  pinMode(RED_PIN, OUTPUT);
  pinMode(OR_PIN, OUTPUT);
  pinMode(GRE_PIN, OUTPUT);
  setupSequence();
  timer.setInterval(600000, software_Reset);
}

//*****************************************************************************************//
void loop() {
  timer.run();
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  digitalWrite(OR_PIN, HIGH);
//  Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

//  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------

//  Serial.print(F("Name: "));

  byte buffer1[18];

  block = 4;
  len = 18;
  String firstName;
  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("Authentication failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("Reading failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  //PRINT FIRST NAME
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      firstName = firstName + char(buffer1[i]);
//      Serial.print(buffer1[i]);
//      Serial.write(buffer1[i]);
    }
  }
//  Serial.print(" ");

  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;
  String lastName;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("Authentication failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("Reading failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
    if(buffer2[i] != 32){
      lastName = lastName + char(buffer2[i]);
  //    Serial.print(buffer2[i] );
//      Serial.write(buffer2[i] );
    }
  }
  digitalWrite(OR_PIN, LOW);
  //----------------------------------------

//  Serial.println(F("\n**End Reading**\n"));
  if(checkAuth(firstName, lastName)){
//    Serial.println("Authorized!");
    executeUnlock();
//    blinkLED(GRE_PIN);
  }else{
    blinkLED(RED_PIN);
  }  
  delay(1000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

bool checkAuth(String first,String last){
  char buffer1[MAX_LEN];
  first.toCharArray(buffer1,MAX_LEN);
  char buffer2[MAX_LEN];
  last.toCharArray(buffer2,MAX_LEN);
  unsigned char* hash=MD5::make_hash(buffer1);
  String firstkey = String(MD5::make_digest(hash, 16));
  hash=MD5::make_hash(buffer2);
  String secondkey = String(MD5::make_digest(hash, 16));
  return (firstkey == authKey1 && secondkey == authKey2);
}
void executeUnlock(){
//  digitalWrite(OR_PIN, HIGH);
  Serial.write('U');
  blinkLED(GRE_PIN);
  delay(1000);
//  digitalWrite(OR_PIN, LOW);
}

void software_Reset(){// Restarts program from beginning but does not reset the peripherals and registers
asm volatile ("  jmp 0");  
}

void blinkLED(int PIN){
  for (uint8_t i = 0; i < 5; i++) {
    digitalWrite(PIN, HIGH); // turn the LED on (HIGH is the voltage level) 
    delay(50); // wait for a second 
    digitalWrite(PIN, LOW); // turn the LED off by making the voltage LOW 
    delay(50);
  }
}

void setupSequence(){
  for (uint8_t i = 5; i < 8; i++) {
    digitalWrite(i, HIGH); // turn the LED on (HIGH is the voltage level) 
    delay(50); // wait for a second 
    digitalWrite(i, LOW); // turn the LED off by making the voltage LOW 
    delay(50);
  }
}

//*****************************************************************************************//
