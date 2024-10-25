#include <Servo.h>

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
  // Multiple attempts to verify sensor
  for (int i = 0; i < 3; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    
    digitalWrite(trigPin, LOW);
    
    // Using longer timeout for more reliable detection
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
  myservo.write(0);
  delay(500);
  myservo.write(10);
  delay(500);
  return myservo.attached();
}

float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  
  // Check for invalid readings
  if (duration == 0) {
    return -1;  // Indicate invalid reading
  }
  
  return (duration * 0.034) / 2;
}

int calculateFillPercentage(float distance) {
  if (distance >= 30) return 0;
  if (distance <= 5) return 100;
  return (int)(100 - ((distance - 5) * 100) / 25);
}

void setup() {
  Serial.begin(9600);
  delay(1000);  // Give sensors time to initialize
  
  // Pin setup
  pinMode(binLidTriggerPin, OUTPUT);
  pinMode(binLidEchoPin, INPUT);
  pinMode(binStoreTriggerPin, OUTPUT);
  pinMode(binStoreEchoPin, INPUT);
  
  // Initial state for trigger pins
  digitalWrite(binLidTriggerPin, LOW);
  digitalWrite(binStoreTriggerPin, LOW);
  
  // Verify connections
  Serial.println("\n=== Checking Sensor Connections ===");
  
  // Check lid sensor
  Serial.print("Lid Sensor (Pins ");
  Serial.print(binLidTriggerPin);
  Serial.print("/");
  Serial.print(binLidEchoPin);
  Serial.print("): ");
  lidSensorWorking = verifyUltrasonicSensor(binLidTriggerPin, binLidEchoPin);
  Serial.println(lidSensorWorking ? "Connected" : "NOT CONNECTED");
  
  // Check store sensor
  Serial.print("Store Sensor (Pins ");
  Serial.print(binStoreTriggerPin);
  Serial.print("/");
  Serial.print(binStoreEchoPin);
  Serial.print("): ");
  storeSensorWorking = verifyUltrasonicSensor(binStoreTriggerPin, binStoreEchoPin);
  Serial.println(storeSensorWorking ? "Connected" : "NOT CONNECTED");
  
  // Check servo
  Serial.print("Servo (Pin ");
  Serial.print(servoPin);
  Serial.print("): ");
  myservo.attach(servoPin);
  servoWorking = verifyServo();
  Serial.println(servoWorking ? "Connected" : "NOT CONNECTED");
  
  Serial.println("================================");
  
  if (!lidSensorWorking || !storeSensorWorking || !servoWorking) {
    Serial.println("\nWARNING: Some components are not connected!");
    Serial.println("Please check the connections and reset the system.");
  }
  
  myservo.write(0);  // Initialize servo position
}

void loop() {
  if (!lidSensorWorking && !storeSensorWorking && !servoWorking) {
    Serial.println("Critical failure: No components working. Please check connections.");
    delay(5000);
    return;
  }

  // Check if we're in the measurement pause period
  if (isMeasurementPaused) {
    if (millis() - measurementPauseTime >= 6000) {
      isMeasurementPaused = false;
      isServoMoving = false;
      if (servoWorking) {
        myservo.write(0);
      }
    }
    return;
  }

  // Measure lid distance if sensor is working
  if (lidSensorWorking) {
    lidDistance = measureDistance(binLidTriggerPin, binLidEchoPin);
    if (lidDistance >= 0) {  // Valid reading
      Serial.print("\nLid Distance: ");
      Serial.print(lidDistance);
      Serial.print(" cm");
    } else {
      Serial.print("\nLid Sensor: Reading Error");
    }
  } else {
    Serial.print("\nLid Sensor: Not Connected");
  }

  // Measure store level if sensor is working and lid is not being operated
  if (storeSensorWorking && !isServoMoving) {
    storeDistance = measureDistance(binStoreTriggerPin, binStoreEchoPin);
    if (storeDistance >= 0) {  // Valid reading
      Serial.print(" | Bin Level: ");
      if (storeDistance > 30) {
        Serial.print("NA");
      } else {
        int fillPercentage = calculateFillPercentage(storeDistance);
        Serial.print(fillPercentage);
        Serial.print("%");
        if (fillPercentage >= 95) {
          Serial.print(" *** BIN FULL - ALERT! ***");
        }
      }
    } else {
      Serial.print(" | Bin Store Sensor: Reading Error");
    }
  } else if (!storeSensorWorking) {
    Serial.print(" | Bin Store Sensor: Not Connected");
  }

  // Lid operation if sensors and servo are working
  if (lidSensorWorking && servoWorking && lidDistance >= 0) {
    if (lidDistance < 5 && !isServoMoving) {
      isServoMoving = true;
      servoStartTime = millis();
      myservo.write(90);
    }

    if (isServoMoving && (millis() - servoStartTime >= 500)) {
      measurementPauseTime = millis();
      isMeasurementPaused = true;
      myservo.write(180);
      Serial.println("Lid Open");
      delay(2000);
      myservo.write(0);
      Serial.println("Lid CLosed");
      delay(2000);
    }
  }

  delay(100);
}