#include <SPI.h>
#include <MFRC522.h>
#include <Bounce2.h>

#define RST_PIN         9
#define SS_PIN          10
#define BUTTON1         5
#define BUTTON2         4
#define BUTTON3         3
#define LED1            8
#define LED2            7
#define LED3            6
#define LEDerror        2

byte LED[] = {LED1, LED2, LED3};
int num_cards = 3;
byte cards[3][16] = {{0x00, 0x01, 0x53, 0x44, // SD-CLR , Clear move
                      0x00, 0x00, 0x00, 0x99, 
                      0x00, 0x05, 0x00, 0x00,
                      0x00, 0x02, 0x00, 0x99,},

                      {0x00, 0x01, 0x53, 0x44, // SD , Slow
                      0x00, 0x00, 0x00, 0x01,
                      0x00, 0x05, 0x00, 0x00,
                      0x00, 0x02, 0x00, 0x99,},
                      
                     {0x00, 0x01, 0x46, 0x55, // FU , Stop
                      0x00, 0x00, 0x00, 0x01, 
                      0x00, 0x05, 0x00, 0x00,
                      0x00, 0x02, 0x00, 0x99,}};

//static unsigned long ledCameOn = 0;
byte sector         = 1;
byte blockAddr      = 4;
byte trailerBlock   = 7;
byte buffer[18];
byte size = sizeof(buffer);

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();

void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();

    pinMode(BUTTON1, INPUT);
    pinMode(BUTTON2, INPUT);
    pinMode(BUTTON3, INPUT);

    debouncer1.attach(BUTTON1);
    debouncer1.interval(50);

    debouncer2.attach(BUTTON2);
    debouncer2.interval(50);

    debouncer3.attach(BUTTON3);
    debouncer3.interval(50);

    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
    for (int i = 0; i < num_cards; i++) {
        pinMode(LED[i], OUTPUT);
    }
}

void loop() {
    debouncer1.update();
    debouncer2.update();
    debouncer3.update();

    if (debouncer1.fell()) {
        writeRFID(0);
    }
    if (debouncer2.fell()) {
        writeRFID(1);
    }
    if (debouncer3.fell()) {
        writeRFID(2);
    }

    checkcard();
}

void checkcard() {
    // Check if a new card is present
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        blinkLED(1);
        return; // Return if no card is present or if reading the card UID failed
    }

    // Get the type of the PICC
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

    // Check if the card is of a type that can be read
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        blinkLED(2);
        mfrc522.PICC_HaltA(); // Halt PICC
        return;
    }

    // Authenticate using key A
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        blinkLED(3);
        mfrc522.PICC_HaltA(); // Halt PICC
        return;
    }

    // Read block
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        blinkLED(5);
        mfrc522.PICC_HaltA(); // Halt PICC
        return;
    }

    // Turn off all LEDs initially
    for (int i = 0; i < num_cards; i++) {
        digitalWrite(LED[i], LOW);
    }

    // Compare buffer with stored card data and turn on the corresponding LED
    for (int i = 0; i < num_cards; i++) {
        if (memcmp(buffer, cards[i], 16) == 0) {
            digitalWrite(LED[i], HIGH);
            break; // Stop comparing once a match is found
        }
    }

    // Halt PICC and stop encryption on PCD
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

 
void writeRFID(byte cardnumber) {
    // Check for the presence of a card
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        Serial.println("No card detected");
        return;
    }

    // Load card data to write
    byte dataBlock[16];
    for (byte i = 0; i < 16; i++) {
        dataBlock[i] = cards[cardnumber][i];
    }

    // Authenticate using the key for the block
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Authentication failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Write data to the block
    status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Write failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    Serial.println("Write successful");

    // Halt PICC and stop encryption on PCD
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

void blinkLED(int blinks) {
    for (int i = 0; i < blinks; i++) {
        digitalWrite(LEDerror, HIGH); // Turn on the LED
        delay(500);                   // Wait for 500 milliseconds
        digitalWrite(LEDerror, LOW);  // Turn off the LED
	delay(500);
        }
    }

    // Wait for 2 seconds after completing all blinks
    delay(2000);
}
