// Including libraries for wifi connection 
#include <ESP8266WiFi.h>

// Including libraries for http request and detabase connection 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <detaBaseESP8266.h>
#include "certs.h"

// including libraries for rfid 
#include <SPI.h>
#include <MFRC522.h>

// including libraries for servo
#include <Servo.h>

// detabase details 
// const char* detaKey = "a0bcgx8kxv8_2nU3RUY7M8FCMcDjrEi9pE5DA7UzoG5Q";
// const char* detaID = "a0bcgx8kxv8";
// const char* detaBaseName = "animal_db";

// detabase details 
const char* detaKey = "d02a8qybmmx_eHsNzWDTvs3qetEdk64vy41z1jpzZivQ";
const char* detaID = "d02a8qybmmx";
const char* detaBaseName = "Main_DB";

// wifi credentials 
 //String ssid = "Arnob";
 //String password = "01984933215";

String ssid = "phone";
String password = "asdasd123";

// String ssid = "Silent World";
// String password = "redgreen";

// String ssid = "Hercule Poirot";
// String password = "ab1234cd";

// defining additional variables
short int MAX_SERIAL_READ_WAIT_TIME = 60; // in seconds

// defining pin connections for the rfid 
#define RST_PIN 4  // RST-PIN for RC522 - RFID - SPI - Modul GPIO4
#define SS_PIN  2  // SDA-PIN for RC522 - RFID - SPI - Modul GPIO2
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

// define pin connection for the buzzer
#define BUZZER D0

// dfine pin connection for servo control
#define SERVO D1

// setting up gate open time
int GATE_OPEN_TIME = 5; // in seconds

// create detabase connection
detaBaseESP8266 detabase(detaKey, detaID, detaBaseName);

// create digital certification for secure connection
X509List cert(cert_DigiCert_Global_Root_CA);

// create servo object to control a servo
Servo myservo;

// ring buzzer
void ringBuzzer() {
  digitalWrite(BUZZER, HIGH);
  delay(200); 
  digitalWrite(BUZZER, LOW);
}

// read RFID TAG
String readRFIDTag() {
  static bool printed = false;
  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // ring buzzer
    ringBuzzer();

    // Read the UID of the tag
    String key = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      key += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      key += String(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println("RFID tag detected. UID: " + key);
    printed = false;
    delay(10);
    return key;
  } else {
    if (printed == false) {
      Serial.println("Please place the RFID card near to the scanner to checkout!");
      printed = true;
    }
    delay(500);
    return "";
  }
}

String readSerialInput(String message) {
  String serialData = "";
  int timer = 0;
  Serial.print(message);

  while(timer < MAX_SERIAL_READ_WAIT_TIME*1000) {
    if (Serial.available() > 0) {
      serialData = Serial.readStringUntil('\n');
      break;
    } else {
      timer = timer + 10;
      delay(10);
    }
  }

  if(serialData == "") {
    Serial.println();
    Serial.println("Max wait time out!");
    serialData = "timeout_fail";
  } else {
    Serial.println(serialData);
  }
  timer = 0;
  return serialData;
}

DynamicJsonDocument readData(String key, int checkOnly = 0){
  // Build the URL for the specific item
  String url = "https://database.deta.sh/v1/" + String(detaID) + "/" + String(detaBaseName) + "/items/" + String(key);

  // Initialize the HTTP client and set headers
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", String(detaKey));

  // Send the GET request
  int httpResponseCode = http.GET();
  // Serial.println(httpResponseCode);

  // Create a JSON buffer
  DynamicJsonDocument doc(1024); // defines the stream size

  // Create an array of items
  // JsonArray items = doc.createNestedArray("items");

  // Create the first item and add it to the array
  // JsonObject item1 = items.createNestedObject();

  DynamicJsonDocument item1(1024);

  if (checkOnly == 1) {
    if (httpResponseCode == 200) {
      item1["key"]              = "keyExists";
    } else {
      item1["key"]              = "keyUnique";
    }
    return item1;
  } else {
    // Check the response code
    if (httpResponseCode == 200) {
      String response = http.getString();
      // Serial.println("HTTP Response code: " + String(httpResponseCode));
      // Serial.println("Response: " + response);
      // Parse the JSON response if needed
      // DynamicJsonDocument doc(1024);
      // deserializeJson(doc, response);

      DeserializationError error = deserializeJson(doc, response);

      if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
      }

      // Serial.println("Response: " + response);

      // Access data using doc
      item1["key"]              = doc["key"];
      item1["owner_name"]       = doc["owner_name"];
      item1["contact"]          = doc["contact"];
      item1["district"]         = doc["district"];
      item1["cattle_age"]       = doc["cattle_age"];
      item1["health_condition"] = doc["health_condition"];
      item1["cattle_price"]     = doc["cattle_price"];
      item1["hasil_amount"]     = doc["hasil_amount"];
      item1["hasil_paid"]       = doc["hasil_paid"];
      item1["authorized"]       = doc["authorized"];

    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
    return item1;
  }
}

void clearScreen(int screen_space = 5) {
  for(int i =0; i< screen_space; i++) {
    Serial.println();
  }
}

void showDetails(DynamicJsonDocument item1) {
  clearScreen(2);
  Serial.println("------------------------------------");
  Serial.print("Owner Name: ");
  Serial.println(String(item1["owner_name"]));
  Serial.print("Origin: ");
  Serial.println(String(item1["district"]));
  Serial.print("Age: ");
  Serial.println(String(item1["cattle_age"]));
  Serial.print("Health Condition: ");
  Serial.println(String(item1["health_condition"]));
  Serial.print("Stall No: ");
  Serial.println(String(item1["stall_no"]));
  Serial.print("Hasil Paid: ");
  Serial.println(String(item1["hasil_paid"]));
  Serial.print("Authorized for Checkout: ");
  Serial.println(String(item1["authorized"]));
  Serial.println("------------------------------------");
  clearScreen(2);
}

void openGate() {
  int pos;

  for (pos = 0; pos <= 180; pos += 1) {  // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);  // tell servo to go to position in variable 'pos'
    delay(5);           // waits 15ms for the servo to reach the position
  }

  delay(GATE_OPEN_TIME*1000);   

  for (pos = 180; pos >= 0; pos -= 1) {  // goes from 180 degrees to 0 degrees
    myservo.write(pos);                  // tell servo to go to position in variable 'pos'
    delay(5);                           // waits 15ms for the servo to reach the position
  }
}

void checkout() {
  String key = readRFIDTag();

  DynamicJsonDocument item1(1024);

  if (key != "") {
    item1 = readData(key, 0);
    
    if(String(item1["key"]) == "null") {
      clearScreen(2);
      Serial.println("No such Entry! Please contact the hasil booth!");
      clearScreen(2);
      return;
    } else {
      showDetails(item1);
      if(String(item1["hasil_paid"]) == "yes"){
        Serial.println("Hasil is already paid for this cattle.");
        Serial.println("You're authorized to checkout.");
        Serial.println("Thanks for using our automated system!");
        clearScreen(2);
        openGate();
      } else {
        Serial.println("Sorry, you're not authorized to checkout!");
        ringBuzzer();
        Serial.println("Please contact the hasil booth!");
        clearScreen(2);
      }
      return;
    }
  } else {
    return;
  }
}

void setup() {
  // setting the baud-rate
  Serial.begin(9600);

  // setting up max timeout time for serial input
  Serial.setTimeout(MAX_SERIAL_READ_WAIT_TIME*1000);

  // setting up pin modes for buzzer
  pinMode(BUZZER, OUTPUT);

  // setting up pin modes for servo
  pinMode(SERVO, OUTPUT);

  // attaches the servo to the servo object
  myservo.attach(SERVO);
  myservo.write(0);
  
  // For SSL, UTC+6 Dhaka
  configTime(6*3600, 0, "pool.ntp.org");

  WiFiClientSecure client;
  client.setTrustAnchors(&cert);

  delay(100);
  
  // connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("...");
  }
  Serial.println();
  Serial.println("Wifi Connected!");

  // setting up the RFID reader
  Serial.println("Setting up RFID Reader...");
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  Serial.println("Set up RFID Reader completed!");
}

void loop() {
  checkout(); // perform checkout
  delay(100);
}
