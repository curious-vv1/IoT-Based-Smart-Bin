#include <Servo.h>

Servo myservo;
int binLidTriggerPin = 8;
int binLidEchoPin = 12;
int binStoreTriggerPin = 4;
int binStoreEchoPin = 7;
float lidDistance;
float storeDistance;
float duration;
bool isServoMoving = false;
unsigned long servoStartTime = 0;
unsigned long measurementPauseTime = 0;
bool isMeasurementPaused = false;

// Function to measure distance from ultrasonic sensor
float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  return (duration * 0.034) / 2;
}

// Function to calculate bin fill percentage
int calculateFillPercentage(float distance) {
  if (distance >= 30) return 0;
  if (distance <= 5) return 100;
  
  // Linear mapping from 5-30cm to 100-0%
  return (int)(100 - ((distance - 5) * 100) / 25);
}

void setup() {
  Serial.begin(9600);
  pinMode(binLidTriggerPin, OUTPUT);
  pinMode(binLidEchoPin, INPUT);
  pinMode(binStoreTriggerPin, OUTPUT);
  pinMode(binStoreEchoPin, INPUT);
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

  // Measure lid distance
  lidDistance = measureDistance(binLidTriggerPin, binLidEchoPin);
  Serial.print("\nLid Distance: ");
  Serial.print(lidDistance);
  Serial.print(" cm");

  // Only measure bin store level if lid is not being operated
  if (!isServoMoving) {
    // Measure store distance
    storeDistance = measureDistance(binStoreTriggerPin, binStoreEchoPin);
    
    Serial.print(" | Bin Level: ");
    if (storeDistance > 30) {
      Serial.print("NA");
    } else {
      int fillPercentage = calculateFillPercentage(storeDistance);
      Serial.print(fillPercentage);
      Serial.print("%");
      
      // Alert if bin is full (95% or more)
      if (fillPercentage >= 95) {
        Serial.print(" *** BIN FULL - ALERT! ***");
      }
    }
  }

  // Check if object is closer than 5cm and servo isn't already moving
  if (lidDistance < 5 && !isServoMoving) {
    isServoMoving = true;
    servoStartTime = millis();
    myservo.write(90);  // Move servo to 90 degrees
  }

  // Check if servo has been moving for 500ms
  if (isServoMoving && (millis() - servoStartTime >= 500)) {
    isMeasurementPaused = true;
    measurementPauseTime = millis();
    // Start the servo movement sequence
    myservo.write(180);
    delay(2000);
    myservo.write(0);
    delay(2000);
  }

  delay(100);  // Small delay between measurements
}