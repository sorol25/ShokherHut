#include <Stepper.h>
#define STEPS 2048
Stepper stepper(STEPS,8,10,9,11);
int pirPin = 3;  //pir
void setup() {
 Serial.begin(9600);
 pinMode(pirPin, INPUT); // Set the PIR sensor pin as input
}

void loop() {
    int motionState = digitalRead(pirPin); // Read the PIR sensor's output
     Serial.println(motionState);
 if (motionState == HIGH) {
    stepper.setSpeed(12);
    stepper.step(-2090);
  delay(200);
}
}
