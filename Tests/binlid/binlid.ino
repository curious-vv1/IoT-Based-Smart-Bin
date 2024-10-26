#include <Servo.h>

Servo myservo;
int triggerPin = 8;
int echoPin = 12;
float distance;
float duration;
bool isServoMoving = false;
unsigned long servoStartTime = 0;
unsigned long measurementPauseTime = 0;
bool isMeasurementPaused = false;

void setup() {
  Serial.begin(9600);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  myservo.attach(9);
  myservo.write(0);  // Initialize servo at 0 degrees
}

void loop() {
  // Check if we're in the measurement pause period
  if (isMeasurementPaused) {
    if (millis() - measurementPauseTime >= 6000) {  // 6 seconds passed
      isMeasurementPaused = false;
      isServoMoving = false;
      myservo.write(0);  // Ensure servo is back at 0
    }
    return;  // Skip the rest of the loop while paused
  }

  // Measure distance using ultrasonic sensor
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(10);
  
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  
  digitalWrite(triggerPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.034) / 2;
  
  Serial.print("\n Distance: ");
  Serial.print(distance);
  Serial.print(" cm");

  // Check if object is closer than 5cm and servo isn't already moving
  if (distance < 5 && !isServoMoving) {
    isServoMoving = true;
    servoStartTime = millis();
    myservo.write(90);  // Move servo to 90 degrees
  }

  // Check if servo has been moving for 500ms
  if (isServoMoving && (millis() - servoStartTime >= 500)) {
    isMeasurementPaused = true;
    measurementPauseTime = millis();
    // Start the servo movement sequence
    myservo.write(90);
    delay(2000);
    myservo.write(0);
    delay(2000);
  }

  delay(100);  // Small delay between measurements
}