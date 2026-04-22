#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

// Fingerprint for demo URL, expires on Monday, October 2, 2023 at 2:16:17 PM, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x22, 0xD6, 0x3F, 0x7A, 0xCA, 0x1E, 0x3B, 0x04, 0x40, 0x02, 0xA1, 0xAF, 0x49, 0xB4, 0x02, 0x8E, 0x8D, 0x0E, 0xF9, 0x43};
//22 D6 3F 7A CA 1E 3B 04 40 02 A1 AF 49 B4 02 8E 8D 0E F9 43

//for a - 214 - 51 DB 50 20
//for b - 213 - 23 71 60 0E
//tag - A9 E0 BB 79

byte authorizedCard1[4] = {0x51, 0xDB, 0x50, 0x20}; // Replace with your authorized card's UID
byte authorizedCard2[4] = {0x23, 0x71, 0x60, 0x0E}; // Replace with your authorized card's UID


#define RST_PIN  D3     // Configurable, see typical pin layout above
#define SS_PIN   D4     // Configurable, see typical pin layout above
#define BUZZER   D2     // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;  
ESP8266WiFiMulti WiFiMulti;
MFRC522::StatusCode status;      

/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  

/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];

String data2;
const String data1 = "https://script.google.com/macros/s/AKfycbxF9IdsIx4kytc5S0qRTfZbeNx6U7ZwtJQ2HyMfjKEo8TaGqseG7iwmGOjogsz1X7GULQ/exec?Hasil=";
String rfidData; 

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool isCardAuthorized(byte* scannedUID) {
  // Compare the scannedUID with authorized card IDs
  if (memcmp(scannedUID, authorizedCard1, 4) == 0 || memcmp(scannedUID, authorizedCard2, 4) == 0) {
    return true; // Card is authorized
  } else {
    return false; // Card is not authorized
  }
}

int x;
float y;


String getUserInput() {
  String userInput;
  while (Serial.available() == 0) {
    // Wait for user input
  }
  delay(5000); // Give some time for complete input
  userInput = Serial.readString();
  userInput.trim(); // Remove leading and trailing whitespaces
  return userInput;
}

void setup() 
{
  /* Initialize serial communications with the PC */
  Serial.begin(9600);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) 
  {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);

  // Connect to Wi-Fi
  while (!isWiFiConnected()) {
    Serial.print("Connecting to Wi-Fi...");
    WiFi.begin("Arnob", "01984933215"); // Replace with your Wi-Fi name and password
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }

  // Wi-Fi connected
  Serial.println("Connected to Wi-Fi!");
  Serial.println("IP address: " + WiFi.localIP().toString());


  /* Set BUZZER as OUTPUT */
  pinMode(BUZZER, OUTPUT);
  /* Initialize SPI bus */
  SPI.begin();
}

void loop()
{
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

   // Check if the scanned card is authorized
  if (!isCardAuthorized(mfrc522.uid.uidByte)) {
    Serial.println("Unauthorized card. Access denied.");
    delay(2000);
    return;
  }

  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock(blockNum, readBlockData);
  /* If you want to print the full memory dump, uncomment the next line */
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  
  Serial.println("RFID Data:");
rfidData = ""; // Initialize the 'rfidData' variable as an empty string
for (int j = 0; j < 3; j++)
{
  char currentChar = readBlockData[j]; // Get the current character from 'readBlockData'
  Serial.write(currentChar); // Print the character to Serial monitor
  rfidData += currentChar; // Append the character to the 'rfidData' string
}
Serial.println(); // Print a new line to separate the output

// 'rfidData' now contains the RFID data as a string
Serial.print("RFID Data stored in 'rfidData': ");
Serial.println(rfidData);

   // Print the data read from block
  Serial.println();
  Serial.print(F("Last data in RFID:"));
  Serial.print(blockNum);
  Serial.print(F(" --> "));
  for (int j = 0; j < 16; j++)
  {
    Serial.write(readBlockData[j]);
  }
  Serial.println();
  // Convert the data2 string to handle spaces (replace spaces with "%20")
  data2.replace(" ", "%20");

  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);

  // Read the cow ID from the RFID card and convert it to a String
   String cowId = "Cow ID : ";
  for (int i = 0; i < 4; i++) {
    cowId += String(mfrc522.uid.uidByte[i]);
    if (i < 3) {
      cowId += " ";
    }
  }
  Serial.println(cowId);

  // Ask about the cow price
  Serial.println("Welcome to sokher hut");
  Serial.print("price of your cow:");

  while (Serial.available() == 0)
  {
    // Wait for user input
  }

  x = Serial.parseInt();
  y = x * 0.05;

  Serial.println(x);
  Serial.print("your total hashil:");
  Serial.println(y);

   // Ask if the user wants to pay
  Serial.println("do you want to pay?");
  Serial.println("press yes/no");

  String a = getUserInput();
  Serial.print("Answer :");
  Serial.println(a);

  
/////////////////
  if (isWiFiConnected())
  {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(fingerprint);

    // Append the cow ID as a parameter named 'CowID' in the HTTP request URL
    data2 = data1 + String(a) + "&CowID=" + rfidData;
    data2.trim();
    data2.replace(" ", "%20");
    Serial.println(data2);

    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));
    if (https.begin(*client, (String)data2))
    {
      Serial.print(F("[HTTPS] GET...\n"));
      int httpCode = https.GET();
      if (httpCode > 0)
      {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      }
      else
      {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
      delay(1000);
    }
    else
    {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  ///////////////////////////
  // Halt PICC
    mfrc522.PICC_HaltA();
  // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
  
}
 

void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
  /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
  else
  {
    Serial.println("Authentication success");
  }

  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Block was read successfully");  
  }
}

