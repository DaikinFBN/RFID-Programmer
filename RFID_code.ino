/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 *
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 *
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * This sample shows how to read and write data blocks on a MIFARE Classic PICC
 * (= card/tag).
 *
 * BEWARE: Data will be written to the PICC, in sector #1 (blocks #4 to #7).
 *
 *
 * Typical pin layout used:
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
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 *
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

const int BUTTON1 = 7;//Initialize Pin2 with Button1
const int BUTTON2 = 5;//Initialize Pin4 with Button2
const int BUTTON3 = 3;//Initialize Pin7 with Button3
const int LED1 = 8;//Initialize pin8 for LED1
const int LED2 = 6;//Initialize pin12 for LED2
const int LED3 = 4;//Initialize pin13 for LED3
int curState1 = 0; // To read the button1 state
int curState2 = 0;// To read the button2 state
int curState3 = 0;// To read the button3 state
int LED1state = 0;
int LED2state = 0;
int LED3state = 0;
int lastState1;
int lastState2;
int lastState3;
static unsigned long ledCameOn = 0;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

byte sector         = 1;
byte blockAddr      = 4;
byte trailerBlock   = 7;
MFRC522::StatusCode status;
byte buffer[18];
byte size = sizeof(buffer);

// data of the three cards we use at FBN
byte stopCard[] = { //FU  
    0x00, 0x01, 0x46, 0x55, //  1,  2,   3,  4,
    0x00, 0x00, 0x00, 0x01, //  5,  6,   7,  8,
    0x00, 0x05, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x99,
};
byte fastCard[] = {//SD-CLR
    0x00, 0x01, 0x53, 0x44, //  1,  2,   3,  4,
    0x00, 0x00, 0x00, 0x99, //  5,  6,   7,  8,
    0x00, 0x05, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x99,
};
byte slowCard[] = {//SD
    0x00, 0x01, 0x53, 0x44, //  1,  2,   3,  4,
    0x00, 0x00, 0x00, 0x01, //  5,  6,   7,  8,
    0x00, 0x05, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x99,
};

/**
 * Initialize.
 */
void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
  
  pinMode(BUTTON1, INPUT);//Define Button1 as input pin
  pinMode(BUTTON2, INPUT);//Define Button2 as input pin
  pinMode(BUTTON3, INPUT);//Define Button3 as input pin
  pinMode(LED1, OUTPUT);//Define LED1 as output pin
  pinMode(LED2, OUTPUT);//Define LED2 as output pin
  pinMode(LED3, OUTPUT);//Define LED3 as output pin
  curState1 = digitalRead(BUTTON1);
  curState2 = digitalRead(BUTTON2);
  curState3 = digitalRead(BUTTON3);
  
  Serial.print(F("Ready to read a card"));
  Serial.println(); 
}

/**
 * Main loop.
 */
void loop() {

  if (LED1state == HIGH || LED2state == HIGH || LED3state == HIGH)
  {
    if(millis()-ledCameOn > 5000)
    {
      LED1state = LOW;
      LED2state = LOW;
      LED3state = LOW;
    }
  }

  digitalWrite(LED1, LED1state);
  digitalWrite(LED2, LED2state);
  digitalWrite(LED3, LED3state);

  if (digitalRead(BUTTON1) == HIGH || digitalRead(BUTTON2) == HIGH || digitalRead(BUTTON3) == HIGH){

    status = (MFRC522::StatusCode) mfrc522.PICC_WakeupA(buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_WakeupA() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }

    if ( ! mfrc522.PICC_ReadCardSerial())
      return;
      
      rfid_write_button();
      
  }
  else {
      // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
      if ( ! mfrc522.PICC_IsNewCardPresent())
          return;
  
      // Select one of the cards
      if ( ! mfrc522.PICC_ReadCardSerial())
          return;

      MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
      //    Serial.println(mfrc522.PICC_GetTypeName(piccType));
      
      // Check for compatibility
      if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("This sample only works with MIFARE Classic cards."));
      return;
      }
    
        rfid_read();
  }


    
    mfrc522.PICC_HaltA(); // Halt PICC
    mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
}


void rfid_write_user()
{
  // user input to change the card
  Serial.println(F("Do you wish to change the card?"));
  Serial.println(F("1. FU or stop card"));
  Serial.println(F("2. SD or slow card"));
  Serial.println(F("3. SD-CLR or fast card"));
  Serial.println(F("4. Skip writing"));
  Serial.println();
  while (Serial.available() >= 0) 
  {
    char ans = (char)Serial.read();
    if (ans == '1'){
      verify(stopCard);
      break;
    }
    if (ans == '2'){
      verify(slowCard);
      break;
    }
    if (ans == '3'){
      verify(fastCard);
      break;        
    }
    if (ans == '4'){
      Serial.println(F("Ready to read a new card"));
      Serial.println();
      break; 
    }
  }
}

void rfid_write_button()
{
  lastState1 = curState1;
  lastState2 = curState2;
  lastState3 = curState3;
  curState1 = digitalRead(BUTTON1);//Read button1 state
  curState2 = digitalRead(BUTTON2);//Read button2 state
  curState3 = digitalRead(BUTTON3);//Read button3 state

  if (lastState1 == LOW && curState1 == HIGH)
  {
    if (LED1state == LOW){
      LED1state = HIGH;
      LED2state = LOW;
      LED3state = LOW;
      ledCameOn = millis();
      verify(stopCard);
//      Serial.println("led 1 is on");
    }
  }
  
  if (lastState2 == LOW && curState2 == HIGH)
  {
    if (LED2state == LOW){
      LED2state = HIGH;
      LED1state = LOW;
      LED3state = LOW;
      ledCameOn = millis();
      verify(fastCard);
//      Serial.println("led 2 is on");
    }
  }
  
  if (lastState3 == LOW && curState3 == HIGH)
  {
    if (LED3state == LOW){
      
      verify(slowCard);
      LED3state = HIGH;
      LED2state = LOW;
      LED1state = LOW;
      ledCameOn = millis();
//      Serial.println("led 3 is on");
    }
  }
}

void rfid_read()
{
//  Serial.print("read read");
    size = sizeof(buffer);
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    
    // Read data from the block
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();
    Serial.println();

    Serial.print(F("Card is ")); // checks the read card to any of the three known card
    
    if (buffer[2] == stopCard[2]){
      Serial.println(F("FU or a stop card"));
      Serial.println(); 
      LED1state = HIGH;
      LED2state = LOW;
      LED3state = LOW;
      ledCameOn = millis();
    }
    if (buffer[2] == fastCard[2] && buffer[7] == fastCard[7]){
      Serial.println(F("SD-CLR or a fast move card"));
      Serial.println(); 
      LED2state = HIGH;
      LED1state = LOW;
      LED3state = LOW;
      ledCameOn = millis();
    }
    if (buffer[2] == slowCard[2] && buffer[7] == slowCard[7]){
      Serial.println(F("SD or a slow move card"));
      Serial.println(); 
      LED3state = HIGH;
      LED2state = LOW;
      LED1state = LOW;
      ledCameOn = millis();
    }
}
  
void verify(byte *dataBlock){

    size = sizeof(buffer);
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print("verify read");
      // Read data from the block (again, should now be what we have written)
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }

    // Check that data in block is what we have written
    // by counting the number of bytes that are equal
    Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
        // Compare buffer (= what we've read) with dataBlock (= what we've written)
        if (buffer[i] == dataBlock[i])
            count++;
    }
    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
    if (count == 16) {
        Serial.println(F("Card has been wrote sucessfully"));
    } else {
        Serial.println(F("Failure, no match :-("));
        Serial.println(F("  perhaps the write didn't work properly..."));
    }
    Serial.println();
    Serial.println(F("Ready to read a new card"));
    Serial.println();

}


/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
