#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2         //temperature
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int duration = 0;
int distance = 0;
float Celsius = 0;
float smokesensorValue;

int rain =A0;
int LDR_s =A1;
int smoke = A2;
const int relayPin = A3;
const int trigPin = A4;
const int echoPin = A5;
int servoPin = 3;  //shad
const int motorPin1 = 4; //fan
int LDR = 5; //LDR light
int buzzer_light = 7;
Servo myservo,exitservo,fireservo;

void setup() {
  Serial.begin(9600);
  pinMode(servoPin, OUTPUT);
  pinMode(9 , OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(LDR, OUTPUT);
  pinMode(buzzer_light, OUTPUT);
  pinMode(echoPin, INPUT);
   pinMode(smoke, INPUT);
  myservo.attach(servoPin);
  exitservo.attach(9);
  fireservo.attach(6);
  myservo.write(0); //shade on
  Serial.println(" Sensor's warming up !!!! ");
  delay(3000);

}

void loop() {
    int rainsensorValue = analogRead(rain);
    smokesensorValue = analogRead(smoke);
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);  // Increase the delay here, e.g., 5 microseconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = duration / 58.2;
    Serial.print("distance:");
    Serial.print(distance);
    Serial.print(" ");

    float LDR_Sensor_Value = analogRead(LDR_s);
    Serial.print("LDR : ");
    Serial.print(LDR_Sensor_Value);
    Serial.print(" ");

    sensors.requestTemperatures();
    Celsius = sensors.getTempCByIndex(0);
    Serial.print(Celsius);
    Serial.print(" C  ");
if (rainsensorValue < 900 || Celsius > 40) {
  myservo.write(60); //shade off
  if (rainsensorValue < 900 ) {
    Serial.print("!!!! Warning Rain detected !!!!");
  } 
   else if (Celsius > 40) {
    digitalWrite(motorPin1, HIGH);      //fan on                 //shade off
  } 
}
  else {
    myservo.write(0);       //shade on
    digitalWrite(motorPin1, LOW); 
  }

 if (distance <= 4 )       //  (turn off pump)
  {
    digitalWrite(relayPin, LOW);
  } 
  else if (distance <= 8)     // Water level below (turn on )
  {
    digitalWrite(relayPin, HIGH);
  }
   else                     //turn off
  {
    digitalWrite(relayPin, LOW);
  } 


  if(LDR_Sensor_Value>=0 && LDR_Sensor_Value<=30)
  {
      analogWrite(LDR, 253);            //light brightness 
  }
     else if(LDR_Sensor_Value<=45)
  {
      analogWrite(LDR, 190);
  }
     else if(LDR_Sensor_Value<=60)
  {
      analogWrite(LDR, 150);
  }
  else if(LDR_Sensor_Value<=75)
  {
      analogWrite(LDR, 110);
  }
   else if(LDR_Sensor_Value<=90)
  {
      analogWrite(LDR, 70);
  }
    else if(LDR_Sensor_Value<=105)
  {
      analogWrite(LDR, 40);
  }
  else {
  analogWrite(LDR, 0);
  }

  if (smokesensorValue > 130) {
    digitalWrite(buzzer_light, HIGH); // Smoke condition
    Serial.print("!!!! Warning smoke detected !!!!");
    exitservo.write(90);
    fireservo.write(90);
   } 
   else {
          digitalWrite(buzzer_light, LOW);
          exitservo.write(0);
          fireservo.write(0);
   }
 Serial.print(" smoke value: ");
 Serial.print(smokesensorValue);
 Serial.println();
 delay(1000);
}
