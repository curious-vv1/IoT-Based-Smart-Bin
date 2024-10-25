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

bool binStatus = true;  // true = active, false = inactive
int binHeight = 30;  


// Function to initialize default bin configuration
void initializeDefaultConfig() {
    Serial.println("Initializing default configuration...");
    // Set default values
    binStatus = true;      // Active by default
    binHeight = 30;        // 30cm default height
    binFillPercentage = 0; // Start empty
    
    // Create initial configuration in Firebase
    StaticJsonDocument<256> docOutput;
    docOutput["binFilled"] = binFillPercentage;
    docOutput["lidDistance"] = 0;  // Initial lid distance
    docOutput["binLidSensor"] = lidSensorWorking ? "connected" : "disconnected";
    docOutput["binStoreSensor"] = storeSensorWorking ? "connected" : "disconnected";
    docOutput["lid"] = "closed";   // Initial lid state
    docOutput["servo"] = servoWorking ? "connected" : "disconnected";
    docOutput["status"] = binStatus;
    docOutput["binHeight"] = binHeight;

    String output;
    serializeJson(docOutput, output);
    
    // Send to Firebase
    fb.setJson("bin1", output);
    Serial.println("Default configuration initialized in Firebase");
}

// Improved function to retrieve bin configuration
void getBinConfiguration() {
    Serial.println("Retrieving bin configuration...");
    String input = fb.getJson("bin1");
    
    if (input == "null" || input.length() == 0) {
        Serial.println("No existing configuration found");
        initializeDefaultConfig();
        return;
    }
    
    StaticJsonDocument<256> docInput;
    DeserializationError error = deserializeJson(docInput, input);
    
    if (error) {
        Serial.print("Firebase data parsing failed: ");
        Serial.println(error.c_str());
        initializeDefaultConfig();
        return;
    }

    // Load configuration values, using existing values as fallback
    binStatus = docInput.containsKey("status") ? 
                docInput["status"].as<bool>() : true;
    
    binHeight = docInput.containsKey("binHeight") ? 
                docInput["binHeight"].as<int>() : 30;
    
    binFillPercentage = docInput.containsKey("binFilled") ? 
                        docInput["binFilled"].as<int>() : 0;

    Serial.println("Configuration loaded:");
    Serial.print("Status: "); Serial.println(binStatus ? "Active" : "Inactive");
    Serial.print("Bin Height: "); Serial.println(binHeight);
    Serial.print("Fill Percentage: "); Serial.println(binFillPercentage);
}


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
  if (distance >= binHeight) return 0;
  if (distance <= 5) return 100;
  return (int)(100 - ((distance - 5) * 100) / (binHeight - 5));
}

// Send data to Firebase
void updateFirebase() {
  StaticJsonDocument<256> docOutput;
  docOutput["binFilled"] = binFillPercentage;
  docOutput["lidDistance"] = lidDistance;
  docOutput["binLidSensor"] = lidSensorWorking ? "connected" : "disconnected";
  docOutput["binStoreSensor"] = storeSensorWorking ? "connected" : "disconnected";
  docOutput["lid"] = isServoMoving ? "open" : "closed";
  docOutput["servo"] = servoWorking ? "connected" : "disconnected";
  docOutput["status"] = binStatus;
  docOutput["binHeight"] = binHeight;

  String output;
  serializeJson(docOutput, output);
  fb.setJson("bin1", output);
  Serial.println("Smart bin data updated on Firebase.");
}


// Function to check component functionality
bool checkComponents() {
  if (!lidSensorWorking && !storeSensorWorking && !servoWorking) {
    Serial.println("Critical failure: No components working.");
    delay(5000);
    return false;
  }
  return true;
}

// Function to handle lid distance measurement
void measureLidDistance() {
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
}

// Function to perform lid opening operation
void operateLid() {
  isServoMoving = true;
  isMeasurementPaused = true;
  measurementPauseTime = millis();

  myservo.write(180);        // Open lid
  delay(4000);               // Wait for garbage to be thrown in
  updateFirebase();          // Update Firebase to show lid open
  Serial.println("Lid is Opened");

  myservo.write(0);          // Close lid
  delay(1000);               // Short delay to ensure lid closes
  Serial.println("Lid is Closed");
  isServoMoving = false;
  
  measureStoreLevel();       // Check bin store level
  updateFirebase();          // Update Firebase with new data
}

// Measure bin store level and update bin fill percentage
void measureStoreLevel() {
  if (storeSensorWorking && !isServoMoving) {
    storeDistance = measureDistance(binStoreTriggerPin, binStoreEchoPin);
    if (storeDistance >= 0) {
      int newFillPercentage = calculateFillPercentage(storeDistance);
      if (newFillPercentage != binFillPercentage) {
        binFillPercentage = newFillPercentage;
        Serial.print("Bin Fill: ");
        Serial.print(binFillPercentage);
        Serial.println("%");
        if (binFillPercentage >= 95) Serial.println(" *** BIN FULL - ALERT! ***");
      }
    } else {
      Serial.println("Bin Store Sensor: Reading Error");
    }
  }
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
  getBinConfiguration();
}

void loop() {
  if (!binStatus) {
    if (millis() % 10000 == 0) {  // Check status every 10 seconds
      getBinConfiguration();
    }
    return;
  }
  if (!checkComponents()) return;

  if (isMeasurementPaused) {
    if (millis() - measurementPauseTime >= 8000) {
      isMeasurementPaused = false;
    }
    return;
  }

  measureLidDistance();

  if (lidSensorWorking && servoWorking && lidDistance < 5 && !isServoMoving) {
    operateLid();
  }

  measureStoreLevel();

  delay(100);
}
