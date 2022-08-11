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
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 * 
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

    byte dataBlock[]    = {
        0x00, 0x01, 'E', 'L', //  1,  2,   3,  4,
        0x00, 0x00, 0x00, 0x06, //  5,  6,   7,  8,
        0x00, 0x05, 0x00, 0x01, //  9, 10, 255, 12,
        0x00, 0x02, 0x00, 0x99  // 13, 14,  15, 16
    };
char input_state =0;
 String inputString = "";         // a string to hold incoming data
char stringComplete = 0;  // whether the string is complete   
/**
 * Initialize.
 */
void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card
    mfrc522.PCD_SetRegisterBitMask(mfrc522.RFCfgReg, (0x07<<4));  // Enhance the MFRC522 Receiver Gain to maximum value of some 48 dB. This increased the range by 1/2"
  inputString.reserve(40);
    // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    //Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
    //Serial.print(F("Using key (for A and B):"));
    //dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
    //Serial.println();
    
    //Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));
    Serial.println(F("Start RFID"));
    //serialinput();
    dump_byte_array2(dataBlock, 16); Serial.println();
    Serial.print(F("input new data: "));
}

/**
 * Main loop.
 */
void loop()
{
  
  if(stringComplete)
  {
    Serial.println(inputString);
    inputString = "";
    stringComplete = false;
    dump_byte_array2(dataBlock, 16); Serial.println();
    Serial.print(F("input new data: "));
  }
  serialinput();
// Serial.println(F("."));
  loop_rfid();
}
 
void loop_rfid() 
{
    //dump_byte_array2(dataBlock, 16); Serial.println();
    //while(1);
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;
    input_state=0;
    // Show some details of the PICC (that is: the tag/card)
    //Serial.print(F("Card UID:"));
    //dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    //Serial.println();
    //Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    //Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return;
    }

    // In this sample we use the second sector,
    // that is: sector #1, covering block #4 up to and including block #7
    byte sector         = 1;
    byte blockAddr      = 4;
//    byte dataBlock[]    = {
//        0x00, 0x00, 'S', 'E', //  1,  2,   3,  4,
//        0x00, 0x00, 0x00, 0x06, //  5,  6,   7,  8,
//        0x00, 0x05, 0x00, 0x00, //  9, 10, 255, 12,
//        0x00, 0x02, 0x00, 0x99  // 13, 14,  15, 16
//    };
    byte trailerBlock   = 7;
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);

    // Authenticate using key A
   // Serial.println(F("Authenticating using key A..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
////////////////////////////////////////
   // Show the whole sector as it currently is
  //  Serial.println(F("Current data in sector:"));
  //  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  //  Serial.println();

    // Read data from the block
    //Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    //Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
 /*   if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    } */
   //////////////////////////////////////////////////////////////////////////////////////////
    //Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();
    //Serial.println();
  //////////////////////////////////////////////////////////////////////////////////////////////
    // Authenticate using key B
   // Serial.println(F("Authenticating again using key B..."));
   // status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
   // if (status != MFRC522::STATUS_OK) {
   //     Serial.print(F("PCD_Authenticate() failed: "));
   //     Serial.println(mfrc522.GetStatusCodeName(status));
   //     return;
   // }

    // Write data to the block
    Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    //dump_byte_array(dataBlock, 16); Serial.println();
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();

    // Read data from the block (again, should now be what we have written)
    //Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    //Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    dump_byte_array2(buffer, 16); Serial.println();
        
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
        Serial.println(F("Success :-)"));
    } else {
        Serial.println(F("Failure, no match :-("));
        Serial.println(F("  perhaps the write didn't work properly..."));
    }
    Serial.println();
        
    // Dump the sector data
   // Serial.println(F("Current data in sector:"));
   // mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    //Serial.println();
/////////////////////////////////////
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
//}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
  Serial.println(F("input new data: "));
} 


void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void dump_byte_array2(byte *buffer, byte bufferSize)
{
  Serial.print("1. line id: ");
  Serial.print(buffer[0] < 0x10 ? " 0" : " ");
  Serial.print(buffer[0], HEX);
 // payload[0] = buffer[0];
  
  Serial.print(buffer[1] < 0x10 ? " 0" : " ");
  Serial.print(buffer[1], HEX);
 // payload[1] = buffer[1];
  Serial.println(" ");
  Serial.print("2. TAG : ");
  Serial.print((char)buffer[2]);
  Serial.print((char)buffer[3]);
 // payload[2] = buffer[2];
 // payload[3] = buffer[3];
   Serial.println(" ");
  Serial.print("3. section : ");
  for (byte i = 4; i < 8; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  Serial.println(" ");
  Serial.print("4. speed : ");
  
  for (byte i = 8; i < 10; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
     
       Serial.println(" ");
  Serial.print("5. junction : ");
  for (byte i = 10; i < 12; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX); 
   // payload[i] = buffer[i];

  }

    Serial.println(" ");
  Serial.print("6. spare : ");
  for (byte i = 12; i < 14; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX); 
   // payload[i] = buffer[i];

  }
   Serial.println(" ");
    Serial.print("7. remark : ");
  for (byte i = 14; i < 16; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX); 
   // payload[i] = buffer[i];

  }
}


void serialinput()
{
  //if(input_state==0)
 // {
  //input_state=1;
  //Serial.println(F("input new data: "));
  while(Serial.available()) 
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    //Serial.write(inChar);
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
  
  if((inputString[0]=='1')&&(inputString[1]==':'))
  {
   dataBlock[0]=(ascii2hex(inputString[2])*0x10)+(ascii2hex(inputString[3]));
   dataBlock[1]=(ascii2hex(inputString[4])*0x10)+(ascii2hex(inputString[5]));
  }

  if((inputString[0]=='2')&&(inputString[1]==':'))
  {
   dataBlock[2]=inputString[2];
   dataBlock[3]=inputString[3];
  }

  if((inputString[0]=='3')&&(inputString[1]==':'))
  {
   dataBlock[4]=(ascii2hex(inputString[2])*0x10)+(ascii2hex(inputString[3]));
   dataBlock[5]=(ascii2hex(inputString[4])*0x10)+(ascii2hex(inputString[5]));
   dataBlock[6]=(ascii2hex(inputString[6])*0x10)+(ascii2hex(inputString[7]));
   dataBlock[7]=(ascii2hex(inputString[8])*0x10)+(ascii2hex(inputString[9]));
  }

  if((inputString[0]=='4')&&(inputString[1]==':'))
  {
   dataBlock[8]=(ascii2hex(inputString[2])*0x10)+(ascii2hex(inputString[3]));
   dataBlock[9]=(ascii2hex(inputString[4])*0x10)+(ascii2hex(inputString[5]));
  }

  if((inputString[0]=='5')&&(inputString[1]==':'))
  {
   dataBlock[10]=(ascii2hex(inputString[2])*0x10)+(ascii2hex(inputString[3]));
   dataBlock[11]=(ascii2hex(inputString[4])*0x10)+(ascii2hex(inputString[5]));
  }

  if((inputString[0]=='6')&&(inputString[1]==':'))
  {
   dataBlock[12]=(ascii2hex(inputString[2])*0x10)+(ascii2hex(inputString[3]));
   dataBlock[13]=(ascii2hex(inputString[4])*0x10)+(ascii2hex(inputString[5]));
  }
  if((inputString[0]=='7')&&(inputString[1]==':'))
  {
   dataBlock[14]=(ascii2hex(inputString[2])*0x10)+(ascii2hex(inputString[3]));
   dataBlock[15]=(ascii2hex(inputString[4])*0x10)+(ascii2hex(inputString[5]));
  }
  
}  

byte ascii2hex(byte in)
{
  byte ret=0;
  switch (in)
  {
    case '0': ret=0; break;
    case '1': ret=1; break;
    case '2': ret=2; break;
    case '3': ret=3; break;
    case '4': ret=4; break;
    case '5': ret=5; break;
    case '6': ret=6; break;
    case '7': ret=7; break;
    case '8': ret=8; break;
    case '9': ret=9; break;
    case 'a': ret=10; break;
    case 'b': ret=11; break;
    case 'c': ret=12; break;
    case 'd': ret=13; break;
    case 'e': ret=14; break;
    case 'f': ret=15; break;
    case 'A': ret=10; break;
    case 'B': ret=11; break;
    case 'C': ret=12; break;
    case 'D': ret=13; break;
    case 'E': ret=14; break;
    case 'F': ret=15; break;
  }
 return ret;
}
