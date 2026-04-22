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
// setting up gate open time
int GATE_OPEN_TIME = 5; // in seconds

// select option to perform
short int selected_option = 4;

// defining pin connections for the rfid 
#define RST_PIN 0  // RST-PIN for RC522 - RFID - SPI - Modul GPIO0 
#define SS_PIN  2  // SDA-PIN for RC522 - RFID - SPI - Modul GPIO2
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

// defining pin connections for transport tracking
#define TRANSPORT1 16
#define TRANSPORT2 9
#define TRANSPORT3 10

// define availability for the transport
bool TRANSPORT1FREE = true;
bool TRANSPORT2FREE = true;
bool TRANSPORT3FREE = true;

// define transport reset time
int TRANSPORTRESETTIME = 10; // in seconds;
int TRANSPORTRESETCOUNTER = 0;

// define pin connection for the buzzer
#define BUZZER D2

// dfine pin connection for servo control
#define SERVO D1

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
  delay(200);
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
      Serial.println("Please place the RFID card near to the scanner!");
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
  return serialData;
}

bool insertData(String key, String owner_name, String contact, String district, String cattle_age, String health_condition, String stall_no, String cattle_price, String hasil_amount, String hasil_paid, String authorized){
  // success flag
  bool success_flag = false;

  // Build the URL for the base
  String url = "https://database.deta.sh/v1/" + String(detaID) + "/" + String(detaBaseName) + "/items";

  // Create a JSON buffer
  DynamicJsonDocument doc(1024); // defining stream size

  // Create an array of items
  JsonArray items = doc.createNestedArray("items");

  // Create the first item and add it to the array
  JsonObject item1 = items.createNestedObject();
  item1["key"] = key;
  item1["owner_name"] = owner_name;
  item1["contact"] = contact;
  item1["district"] = district;
  item1["cattle_age"] = cattle_age;
  item1["health_condition"] = health_condition;
  item1["stall_no"] = stall_no;
  item1["cattle_price"] = cattle_price;
  item1["hasil_amount"] = hasil_amount;
  item1["hasil_paid"] = hasil_paid;
  item1["authorized"] = authorized;

  // Serialize the JSON payload to a string
  String payload;
  serializeJson(doc, payload);

  // Set the Content-Type header to application/json
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", String(detaKey));

  // Send the PUT request with the JSON payload
  int httpResponseCode = http.PUT(payload);

  Serial.println(payload);

  // Check the response code
  if (httpResponseCode > 0) {
    String response = http.getString();
    // Serial.println("HTTP Response code: " + String(httpResponseCode));
    // Serial.println("Response: " + response);
    Serial.println("Successfully added the data to detabase!");
    success_flag = false;
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();
  return success_flag;
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

bool createUpdateData(int update = 0) {
  bool success_flag = false;
  String key = readRFIDTag();
  String confirmation = ""; 
  String owner_name = "";
  String contact = "";
  String district = "";
  String cattle_age = "";
  String health_condition = "";
  String stall_no = "";

  // Create a JSON buffer
  DynamicJsonDocument doc(256);

  // Create an array of items
  // JsonArray items = doc.createNestedArray("items");

  // Create the first item and add it to the array
  // JsonObject item1 = items.createNestedObject();

  DynamicJsonDocument item1(1024);

  if (key != "") {
    if (update == 0){
      item1 = readData(key, 1);

      if (item1["key"] == "keyExists") {
        Serial.print("There is an entry with this card: ");
        Serial.println(key);
        confirmation = readSerialInput("Do you want to replace this? (yes/no) ");
        
        if(confirmation == "no" || confirmation == "timeout_fail") {
          Serial.println("Operation aborted!");
          key = "";
          selected_option = 4;
          return false;
        }
      }
    }

    owner_name = readSerialInput("Enter owner name: ");

    if (owner_name != "timeout_fail") {
      contact = readSerialInput("Enter contact number: ");
    } else {
      selected_option = 4;
      return false;
    }

    if (contact != "timeout_fail") {
      district = readSerialInput("Enter district name: ");
    } else {
      selected_option = 4;
      return false;
    }

    if (district != "timeout_fail") {
      cattle_age = readSerialInput("Enter age of the cattle: ");
    } else {
      selected_option = 4;
      return false;
    }

    if (cattle_age != "timeout_fail") {
      health_condition = readSerialInput("Enter current health of the cattle: ");
    } else {
      selected_option = 4;
      return false;
    }

    if (health_condition != "timeout_fail") {
      stall_no = readSerialInput("Enter available stall no to accommodate the cattle: ");
    } else {
      selected_option = 4;
      return false;
    }

    String cattle_price = "0.0";
    String hasil_amount = "0.0";
    String hasil_paid = "No";
    String authorized = "No";
    
    item1["key"] = key;
    item1["owner_name"] = owner_name;
    item1["contact"] = contact;
    item1["district"] = district;
    item1["cattle_age"] = cattle_age;
    item1["health_condition"] = health_condition;
    item1["stall_no"] = stall_no;
    item1["cattle_price"] = cattle_price;
    item1["hasil_amount"] = hasil_amount;
    item1["hasil_paid"] = hasil_paid;
    item1["authorized"] = authorized;
    showDetails(item1);

    success_flag = insertData(key, owner_name, contact, district, cattle_age, health_condition, stall_no, cattle_price, hasil_amount, hasil_paid, authorized);
    selected_option = 4;
    return success_flag;
  } else {
    return false;
  }
}

bool deleteData() {
  String key = readRFIDTag();
  if (key != "") {
    String confirmation = readSerialInput("Do you really want to delete this entry? (yes/no) ");
    
    if(confirmation == "no" || confirmation == "timeout_fail") {
      Serial.println("Operation aborted!");
      key = "";
      selected_option = 4;
      return false;
    }

    if (confirmation != "timeout_fail") {
      bool isDeleted = detabase.deleteItem(key);

      if (isDeleted) {
        Serial.println("Entry deleted successfully!");
      } else {
        Serial.println("Entry deletion failed!");
      }
    } else {
      Serial.println("Entry deletion timeout failed!");
      selected_option = 4;
      return false;
    }
    selected_option = 4;
    return true;
  } else {
    return false;
  }
}

void availTransport(short int stall_no) {
  String confirmation = "no";
  String selectedTransport = "None";
  String nearbyGate = "1";

  // if (stall_no < 5) {
  //   String nearbyGate = "1";
  // } else if (stall_no < 9) {
  //   String nearbyGate = "2";
  // } else if (stall_no < 13) {
  //   String nearbyGate = "3";
  // } else {
  //   String nearbyGate = "4";
  // }

  confirmation = readSerialInput("Do you want to avail transport service? (yes/no) ");
    
  if(confirmation == "no" || confirmation == "timeout_fail") {
    Serial.println("Operation aborted!");
    return;
  }

  if(digitalRead(TRANSPORT1) == 0 && TRANSPORT1FREE == true) {
    selectedTransport = "Parking 1";
    TRANSPORT1FREE = false;
  } else if(digitalRead(TRANSPORT2) == 0 && TRANSPORT2FREE == true) {
    //selectedTransport = "TRANSPORT2";
    selectedTransport = "Parking 2";
    TRANSPORT2FREE = false;
  } else if(digitalRead(TRANSPORT3) == 0 && TRANSPORT3FREE == true) {
    //selectedTransport = "TRANSPORT3";
    selectedTransport = "Parking 3";
    TRANSPORT3FREE = false;
  } else {
    selectedTransport = "None";
  }

  if (selectedTransport == "None") {
    Serial.println("None of the transport are available. Sorry for your inconveniences!");
    delay(2000);
  } else {
    Serial.print("Car in ");
    Serial.print(selectedTransport);
    Serial.print(" is booked for you! \n");
    //Serial.println(nearbyGate);
    Serial.println("Collect your receipt from here and Show it to the driver to use the transport!");
    Serial.println("Thanks for using our service!");
    delay(5000);
  }

  return;
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

void collectHasil() {
  static bool printed = false;
  if (printed == false) {
    Serial.println("");
    Serial.println("Welcome to shokher hut! Thanks for using our automated system!");
    Serial.println("");

    printed = true;
  }

  String key = readRFIDTag();

  String cattle_price = "0.0";
  float hasil_rate = 0.0;
  String hasil_amount = "0.0";
  String hasil_paid = "No";
  String authorized = "No";

  String confirmation = "no";

  // Create a JSON buffer
  // DynamicJsonDocument doc(256);

  // Create an array of items
  // JsonArray items = doc.createNestedArray("items");

  // Create the first item and add it to the array
  // JsonObject item1 = items.createNestedObject();
  // JsonObject item1;

  DynamicJsonDocument item1(1024);

  if (key != "") {
    item1 = readData(key, 0);
    
    if(String(item1["key"]) == "null") {
      clearScreen(2);
      Serial.println("We apologize for the inconvenience. Try again !");
      clearScreen(2);
      selected_option = 4;
      printed = false;
      return;
    } else {
      // show details for the rfid tag
      showDetails(item1);
      
      // open gate for the customer to pay hasil
      openGate();

      if(String(item1["hasil_paid"]) == "yes"){
        Serial.print("Hasil is already paid for this cattle. Do you still want to pay hasil for this entry again? (");
        Serial.print(key);
        confirmation = readSerialInput(") (yes/no) ");
      
        if(confirmation == "no" || confirmation == "timeout_fail") {
          Serial.println(" Thank you ! ");
          key = "";
          selected_option = 4;
          printed = false;
          return;
        }
      } else {
        Serial.print("Do you really want to pay hasil for this entry? (");
        Serial.print(key);
        confirmation = readSerialInput(") (yes/no) ");
        
        if(confirmation == "no" || confirmation == "timeout_fail") {
          Serial.println("Remember : Paying Hasil is a must !");
          key = "";
          selected_option = 4;
          printed = false;
          return;
        }
      }

      cattle_price = readSerialInput("Enter cattle price: ");

      if (cattle_price != "timeout_fail") {
        hasil_rate = readSerialInput("Enter hasil rate (%): ").toFloat();
      } else {
        selected_option = 4;
        printed = false;
        return;
      }

      hasil_amount = String((cattle_price.toFloat() * hasil_rate)/100, 2);

      Serial.print("Do you agree to pay amount Tk. ");
      Serial.print(hasil_amount);
      Serial.print(" as hasil?");

      confirmation = readSerialInput("(yes/no) ");
      
      if(confirmation == "no" || confirmation == "timeout_fail") {
        Serial.println("Remember : Paying Hasil is a must !");
        key = "";
        selected_option = 4;
        printed = false;
        return;
      }

      item1["cattle_price"] = cattle_price;
      item1["hasil_amount"] = hasil_amount;

      // payment gateway and automatic payment authentication may be added here for full fledged operation
      
      hasil_paid = "yes";
      authorized = "yes";

      bool success_flag = insertData(key, item1["owner_name"], item1["contact"], item1["district"], item1["cattle_age"], item1["health_condition"], item1["stall_no"], cattle_price, hasil_amount, hasil_paid, authorized);

      short int stall_no = String(item1["stall_no"]).toInt();
      availTransport(stall_no);
      selected_option = 4;
      printed = false;
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

  // setting up pin modes for transport tracking
  pinMode(TRANSPORT1, INPUT);
  pinMode(TRANSPORT2, INPUT);
  pinMode(TRANSPORT3, INPUT);

  // setting up pin modes for buzzer
  pinMode(BUZZER, OUTPUT);

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
  
  if (selected_option > 3) {
    clearScreen();
    Serial.println("Select which operation to perform: ");
    Serial.println("1. Add cattle to database");
    Serial.println("2. Update cattle information");
    Serial.println("3. Remove cattle information from the database");
    Serial.println("4. Collect Hasil");
    clearScreen(1);
    selected_option = readSerialInput("Option selected: ").toInt() - 1;
  }
  
  if(selected_option == 0) {
    createUpdateData(0);
  } else if(selected_option == 1) {
    createUpdateData(1);
  } else if(selected_option == 2) {
    deleteData();
  } else if(selected_option == 3) {
    collectHasil();
  } else {
    selected_option = 4;
  }
  
  TRANSPORTRESETCOUNTER++;
  if(TRANSPORTRESETCOUNTER*10 == TRANSPORTRESETTIME*1000) {
    TRANSPORT1FREE = true;
    TRANSPORT2FREE = true;
    TRANSPORT3FREE = true;
  }
  delay(100);
}

/*
void loop() {
  availTransport(6);
  TRANSPORTRESETCOUNTER++;
  Serial.println(TRANSPORTRESETCOUNTER);
  if(TRANSPORTRESETCOUNTER*10 == TRANSPORTRESETTIME*1000) {
    Serial.println("Here");
    TRANSPORT1FREE = true;
    TRANSPORT2FREE = true;
    TRANSPORT3FREE = true;
  }
  delay(100);
}
*/