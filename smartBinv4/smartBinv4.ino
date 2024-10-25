#include <Servo.h>
#include <Firebase.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Servo instance
Servo myservo;

// Pin definitions
const int binLidTriggerPin = 8;
const int binLidEchoPin = 12;
const int binStoreTriggerPin = 4;
const int binStoreEchoPin = 7;
const int servoPin = 9;

// Firebase instance
Firebase fb(REFERENCE_URL);

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
int binFillPercentage = 0;

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
    delay(50);  // Wait before next attempt
  }
  return false;  // Sensor failed all attempts
}

// Function to verify servo connection
bool verifyServo() {
  return myservo.attached();
}

// Measure distance using ultrasonic sensor
float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  return (duration == 0) ? -1 : (duration * 0.034) / 2;
}

// Calculate bin fill percentage based on distance
int calculateFillPercentage(float distance) {
  if (distance >= 30) return 0;
  if (distance <= 5) return 100;
  return (int)(100 - ((distance - 5) * 100) / 25);
}

// Send data to Firebase
void updateFirebase() {
  // Create JSON document to hold bin data
  StaticJsonDocument<256> docOutput;
  docOutput["binFilled"] = binFillPercentage;
  docOutput["lidDistance"] = lidDistance;
  docOutput["binLidSensor"] = lidSensorWorking ? "connected" : "disconnected";
  docOutput["binStoreSensor"] = storeSensorWorking ? "connected" : "disconnected";
  docOutput["lid"] = isServoMoving ? "open" : "closed";
  docOutput["servo"] = servoWorking ? "connected" : "disconnected";

  // Serialize JSON to string and send to Firebase
  String output;
  serializeJson(docOutput, output);
  fb.setJson("bin1", output);
  Serial.println("Smart bin data updated on Firebase.");
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  #if !defined(ARDUINO_UNOWIFIR4)
    WiFi.mode(WIFI_STA);
  #else
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
  #endif
  WiFi.disconnect();
  delay(1000);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("-");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi Connected");
  
  #if defined(ARDUINO_UNOWIFIR4)
    digitalWrite(LED_BUILTIN, HIGH);
  #endif

  // Pin setup
  pinMode(binLidTriggerPin, OUTPUT);
  pinMode(binLidEchoPin, INPUT);
  pinMode(binStoreTriggerPin, OUTPUT);
  pinMode(binStoreEchoPin, INPUT);
  
  digitalWrite(binLidTriggerPin, LOW);
  digitalWrite(binStoreTriggerPin, LOW);

  // Verify connections
  lidSensorWorking = verifyUltrasonicSensor(binLidTriggerPin, binLidEchoPin);
  storeSensorWorking = verifyUltrasonicSensor(binStoreTriggerPin, binStoreEchoPin);
  myservo.attach(servoPin);
  servoWorking = verifyServo();

  // Initialize servo position
  myservo.write(0);

  // Initial Firebase update
  updateFirebase();
}

void loop() {
  if (!lidSensorWorking && !storeSensorWorking && !servoWorking) {
    Serial.println("Critical failure: No components working.");
    delay(5000);
    return;
  }

  if (isMeasurementPaused) {
    if (millis() - measurementPauseTime >= 6000) {
      isMeasurementPaused = false;
      isServoMoving = false;
      if (servoWorking) myservo.write(0);
    }
    return;
  }

  // Measure lid distance if sensor is working
  if (lidSensorWorking) {
    lidDistance = measureDistance(binLidTriggerPin, binLidEchoPin);
    if (lidDistance >= 0) {
      Serial.print("Lid Distance: ");
      Serial.print(lidDistance);
      Serial.println(" cm");
    } else {
      Serial.println("Lid Sensor: Reading Error");
    }
  }

  // Measure store level if sensor is working
  if (storeSensorWorking && !isServoMoving) {
    storeDistance = measureDistance(binStoreTriggerPin, binStoreEchoPin);
    if (storeDistance >= 0) {
      int newFillPercentage = calculateFillPercentage(storeDistance);
      if (newFillPercentage != binFillPercentage) {
        binFillPercentage = newFillPercentage;
        updateFirebase();  // Update Firebase when bin fill percentage changes
        Serial.print("Bin Fill: ");
        Serial.print(binFillPercentage);
        Serial.println("%");
        if (binFillPercentage >= 95) Serial.println(" *** BIN FULL - ALERT! ***");
      }
    } else {
      Serial.println("Bin Store Sensor: Reading Error");
    }
  }

  // Lid operation
  if (lidSensorWorking && servoWorking && lidDistance < 5 && !isServoMoving) {
    isServoMoving = true;
    servoStartTime = millis();
    myservo.write(90);
    delay(500);  // Lid open
    myservo.write(180);
    delay(2000);  // Wait for garbage to be thrown in
    myservo.write(0);
    updateFirebase();  // Update Firebase after lid closes
    Serial.println("Lid Operation Completed");
  }

  delay(100);
}
