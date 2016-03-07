#include <SPI.h>
#include <MFRC522.h>
#include <String.h>
#include "U8glib.h"
#include <Adafruit_NeoPixel.h>

#define SS_PIN 10
#define RST_PIN 5
#define ledGreen 2
#define klingelPin 3

#define NeoPixelDataPin 6
#define NeoPixelPWRPin  7
#define NUMPIXELS       9


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NeoPixelDataPin, NEO_GRB + NEO_KHZ800);

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0); // I2C / TWI

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[3];

bool accessGranted = false;
char accessGrantedFor[20];

byte bekannteKarten[14][4] =
{
  { 0x32, 0x8E, 0x15, 0xDB },   // ID = 100 / Karte
  { 0x04, 0x3E, 0xF1, 0xBA },   // ID = 101 / Karte
  { 0x04, 0xAA, 0x00, 0xBA },   // ID = 102 / Karte
  { 0x04, 0xA7, 0x88, 0x7A },   // ID = 103 / Karte
  { 0x04, 0x78, 0xD2, 0xBA },   // ID = 104 / Karte
  //{ 0x04, 0x55, 0x90, 0x7A },   // ID = 105 / Karte
  
  //{ 0xEE, 0xA6, 0x59, 0x79 },   // ID = 201 / Schlüssel
  //{ 0xAE, 0x80, 0x4B, 0x77 },   // ID = 202 / Schlüssel
  //{ 0xE0, 0x54, 0x67, 0x92 },   // ID = 203 / Schlüssel
  //{ 0x5E, 0xC5, 0x42, 0x77 },   // ID = 204 / Schlüssel
  //{ 0x84, 0x5E, 0xAB, 0x4D },   // ID = 205 / Schlüssel
  //{ 0xCE, 0x65, 0x45, 0x77 },   // ID = 206 / Schlüssel
  //{ 0x3C, 0x64, 0x7D, 0x22 },   // ID = 207 / Schlüssel
  { 0xA4, 0xA3, 0x5F, 0xA7 }    // ID = 208 / Schlüssel
};

char bekannteUser[14][20] =
{
  { "Stefan" },         // ID = 100
  { "Haiko" },          // ID = 101
  { "anderer Tobi" },   // ID = 102
  { "Kalle" },          // ID = 103
  { "Patrick" },        // ID = 104
  //{ "neue 105" },       // ID = 105
  
  //{ "neue 201" },       // ID = 201
  //{ "neue 202" },       // ID = 202
  //{ "neue 203" },       // ID = 203
  //{ "neue 204" },       // ID = 204
  //{ "neue 205" },       // ID = 205
  //{ "neue 206" },       // ID = 206
  //{ "neue 207" },       // ID = 207
  { "Tobi" }            // ID = 208
};

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  pixels.begin(); // This initializes the NeoPixel library.
  pinMode(NeoPixelPWRPin, OUTPUT);
  digitalWrite(NeoPixelPWRPin, LOW);

  pinMode(klingelPin, OUTPUT);
  digitalWrite(klingelPin, 10);

  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledGreen, LOW);

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  u8g.sleepOn();
}

void loop() {

  accessGranted = false;
  memset(accessGrantedFor, 0, sizeof accessGrantedFor);

  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
  {
    return;
  }

  Serial.println("NewCard detected, trie to READ.");
  
  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  
  Serial.print("NewCard type is: ");
  Serial.println(piccType);
  
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_UL {
    ledError();
    return;
  }

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  Serial.print("NewCard Serial Hex: ");
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println("");
  Serial.print("NewCard Serial Dec: ");
  printDec(rfid.uid.uidByte, rfid.uid.size);

  // check if users card id is in the list of VIPs
  for (int y = 0; y <= sizeof(bekannteKarten); y++)
  {
    if (bekannteKarten[y][0] == nuidPICC[0] &&
        bekannteKarten[y][1] == nuidPICC[1] &&
        bekannteKarten[y][2] == nuidPICC[2] &&
        bekannteKarten[y][3] == nuidPICC[3] )
    {
      accessGranted = true;
      strncpy(accessGrantedFor, bekannteUser[y], 20);
      break;
    }
  }

  if (accessGranted == true)
  {    
    accessGranted = false;
    
    Serial.print(accessGrantedFor);
    Serial.println(" will rein!");
    
    ringTheBell();

    //display logic
    u8g.firstPage();
    do {
      draw(accessGrantedFor);
    } while ( u8g.nextPage() );
    
    u8g.sleepOff();
    
    ledSuccess();
  }
  else
  {
    Serial.println(F("ULF - Unbekannte Lebensform."));
    ledError();
  }

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  //needed for display support
  delay(5000);
  u8g.sleepOn();
}

void draw(char name[20]) {
  u8g.setFont(u8g_font_osb18);
  u8g.drawStr( 0, 42, name);
}

void ringTheBell()
{
  digitalWrite(klingelPin, LOW);
  delay(600);
  digitalWrite(klingelPin, 10);
}

void ledSuccess()
{
  digitalWrite(NeoPixelPWRPin, HIGH);
  
  pixels.setPixelColor(0, pixels.Color(0,10,0));
  pixels.setPixelColor(1, pixels.Color(0,10,0));
  pixels.setPixelColor(2, pixels.Color(0,10,0));
  pixels.setPixelColor(3, pixels.Color(0,10,0));
  pixels.setPixelColor(4, pixels.Color(0,10,0));
  pixels.setPixelColor(5, pixels.Color(0,10,0));  
  pixels.setPixelColor(6, pixels.Color(0,10,0));
  pixels.setPixelColor(7, pixels.Color(0,10,0));
  pixels.setPixelColor(8, pixels.Color(0,10,0));  
  pixels.show();
  
  digitalWrite(ledGreen, HIGH);
  
  delay(10000);
  digitalWrite(ledGreen, LOW);
  digitalWrite(NeoPixelPWRPin, LOW);
  pixels.clear();
}

void ledError()
{
  for (int x = 0; x < 3; x++)
  {
    digitalWrite(NeoPixelPWRPin, HIGH);
    
    pixels.setPixelColor(0, pixels.Color(50,0,0));
    pixels.setPixelColor(2, pixels.Color(50,0,0));
    pixels.setPixelColor(4, pixels.Color(50,0,0));
    pixels.setPixelColor(6, pixels.Color(50,0,0));
    pixels.setPixelColor(8, pixels.Color(50,0,0));
    pixels.show();
    
    digitalWrite(ledGreen, HIGH);
    delay(500);
    digitalWrite(ledGreen, LOW);
    digitalWrite(NeoPixelPWRPin, LOW);
    delay(500);
    pixels.clear();
  }
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
