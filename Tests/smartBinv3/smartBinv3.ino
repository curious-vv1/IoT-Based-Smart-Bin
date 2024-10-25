#include "secrets.h"
#include <Servo.h>
#include <Firebase.h>
#include <ArduinoJson.h>

Firebase fb(REFERENCE_URL);

Servo myservo;

// Pin definitions
const int binLidTriggerPin = 8;
const int binLidEchoPin = 12;
const int binStoreTriggerPin = 4;
const int binStoreEchoPin = 7;
const int servoPin = 9;

// Variables for distance measurement
float lidDistance;
float storeDistance;
float duration;
bool isServoMoving = false;
unsigned long servoStartTime = 0;
unsigned long measurementPauseTime = 0;
bool isMeasurementPaused = false;

// Variables for pin status
bool lidSensorWorking = false;
bool storeSensorWorking = false;
bool servoWorking = false;

// Function to verify ultrasonic sensor connection
bool verifyUltrasonicSensor(int trigPin, int echoPin) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 35000);  // Increased timeout
    if (duration > 0) {
      return true;  // Sensor responded
    }
    delay(50);
  }
  return false;  // Sensor failed
}

// Function to verify servo connection
bool verifyServo() {
  myservo.write(0);
  delay(500);
  myservo.write(10);
  delay(500);
  return myservo.attached();
}

// Function to measure distance using ultrasonic sensors
float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
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
  return (int)(100 - ((distance - 5) * 100) / 25);
}

// Function to update Firebase with bin data
void updateFirebase(String path, String key, String value) {
  JsonDocument doc;
  doc[key] = value;
  String output;
  serializeJson(doc, output);
  fb.setJson(path, output);
}

// Setup
void setup() {
  Serial.begin(9600);
  delay(1000);

  // Pin setup
  pinMode(binLidTriggerPin, OUTPUT);
  pinMode(binLidEchoPin, INPUT);
  pinMode(binStoreTriggerPin, OUTPUT);
  pinMode(binStoreEchoPin, INPUT);

  // Servo setup
  myservo.attach(servoPin);
  myservo.write(0);

  // Verify connections
  lidSensorWorking = verifyUltrasonicSensor(binLidTriggerPin, binLidEchoPin);
  storeSensorWorking = verifyUltrasonicSensor(binStoreTriggerPin, binStoreEchoPin);
  servoWorking = verifyServo();

  // Update Firebase with connection statuses
  updateFirebase("bin1/lid", "value", lidSensorWorking ? "connected" : "not connected");
  updateFirebase("bin1/binLidSensor", "value", lidSensorWorking ? "connected" : "not connected");
  updateFirebase("bin1/binStoreSensor", "value", storeSensorWorking ? "connected" : "not connected");
  updateFirebase("bin1/servo", "value", servoWorking ? "connected" : "not connected");
}

void loop() {
  if (!lidSensorWorking && !storeSensorWorking && !servoWorking) {
    Serial.println("No components working. Check connections.");
    delay(5000);
    return;
  }

  // Measure lid distance if sensor is working
  if (lidSensorWorking) {
    lidDistance = measureDistance(binLidTriggerPin, binLidEchoPin);
    Serial.print("Lid Distance: ");
    Serial.print(lidDistance);
    Serial.println(" cm");
  }

  // Measure store distance and calculate fill percentage
  if (storeSensorWorking && !isServoMoving) {
    storeDistance = measureDistance(binStoreTriggerPin, binStoreEchoPin);
    int fillPercentage = calculateFillPercentage(storeDistance);
    Serial.print("Bin Fill Level: ");
    Serial.print(fillPercentage);
    Serial.println("%");

    // Update Firebase with bin fill percentage
    updateFirebase("bin1/binFilled", "value", String(fillPercentage));
  }

  // Servo operation to open lid
  if (lidDistance < 5 && !isServoMoving && servoWorking) {
    isServoMoving = true;
    myservo.write(90);
    updateFirebase("bin1/lid", "value", "opening");
    delay(2000);  // Simulate lid open delay
    myservo.write(0);
    updateFirebase("bin1/lid", "value", "closed");
    isServoMoving = false;
  }

  delay(1000);
}
