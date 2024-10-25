#include <WiFiS3.h>
#include <Firebase.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include "secrets.h"

// Firebase initialization
Firebase firebase(REFERENCE_URL);
StaticJsonDocument<200> jsonBuffer;

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

// System status variables
bool lidSensorWorking = false;
bool storeSensorWorking = false;
bool servoWorking = false;
bool binPowerStatus = true;  // New variable for bin power status
float binHeight = 30.0;      // Default bin height in cm

// Firebase paths
const String BIN_PATH = "bin1";
const String LID_STATUS = "/lid";
const String LID_SENSOR_STATUS = "/binLidSensor";
const String STORE_SENSOR_STATUS = "/binStoreSensor";
const String SERVO_STATUS = "/servo";
const String BIN_POWER = "/bin";
const String BIN_FILLED = "/binFilled";
const String BIN_HEIGHT = "/heightOfBin";

void updateFirebase(const String& path, const String& value) {
  JsonObject data = jsonBuffer.to<JsonObject>();
  data["value"] = value;
  
  String jsonStr;
  serializeJson(data, jsonStr);
  
  firebase.pushString(BIN_PATH + path, jsonStr);
  jsonBuffer.clear();
}

void updateFirebaseInt(const String& path, int value) {
  JsonObject data = jsonBuffer.to<JsonObject>();
  data["value"] = value;
  
  String jsonStr;
  serializeJson(data, jsonStr);
  
  firebase.pushString(BIN_PATH + path, jsonStr);
  jsonBuffer.clear();
}

void checkBinPowerStatus() {
  String powerStatus = firebase.getString(BIN_PATH + BIN_POWER + "/value");
  binPowerStatus = (powerStatus == "on");
}

void checkBinHeight() {
  String heightStr = firebase.getString(BIN_PATH + BIN_HEIGHT + "/value");
  float newHeight = heightStr.toFloat();
  if (newHeight > 0 && newHeight <= 100) {
    binHeight = newHeight;
  }
}

bool verifyUltrasonicSensor(int trigPin, int echoPin) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 35000);
    if (duration > 0) {
      return true;
    }
    delay(50);
  }
  return false;
}

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
  return (duration == 0) ? -1 : (duration * 0.034) / 2;
}

int calculateFillPercentage(float distance) {
  if (distance >= binHeight) return 0;
  if (distance <= 5) return 100;
  return (int)(100 - ((distance - 5) * 100) / (binHeight - 5));
}

void setupWiFi() {
  // Check WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  // Attempt to connect to WiFi network
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print(".");
    delay(1000);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}


bool isFirebasePathEmpty(const String& path) {
  return firebase.getString(path) == "";
}

void createFirebaseData(const String& path, const String& value) {
  if (isFirebasePathEmpty(path)) {
    updateFirebase(path, value);
  }
}

void createFirebaseDataInt(const String& path, int value) {
  if (isFirebasePathEmpty(path)) {
    updateFirebaseInt(path, value);
  }
}

void initializeFirebaseData() {
  Serial.println("Checking Firebase data...");
  
  // Check if the bin data exists
  if (isFirebasePathEmpty(BIN_PATH)) {
    Serial.println("No existing data found for " + BIN_PATH + ". Creating initial data...");
    
    // Set initial values
    createFirebaseData("", "{}");  // Create the bin1 object
    createFirebaseData("/initialized", "true");
    createFirebaseData(BIN_POWER, "on");
    createFirebaseData(LID_STATUS, "closed");
    createFirebaseDataInt(BIN_FILLED, 0);
    createFirebaseData(LID_SENSOR_STATUS, "unknown");
    createFirebaseData(STORE_SENSOR_STATUS, "unknown");
    createFirebaseData(SERVO_STATUS, "unknown");
    createFirebaseDataInt(BIN_HEIGHT, 30);  // Default bin height in cm
    
    Serial.println("Initial data created in Firebase.");
  } else {
    Serial.println("Existing data found in Firebase for " + BIN_PATH);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  delay(1000);
  
  // Setup WiFi
  setupWiFi();

  initializeFirebaseData();
  
  // Pin setup
  pinMode(binLidTriggerPin, OUTPUT);
  pinMode(binLidEchoPin, INPUT);
  pinMode(binStoreTriggerPin, OUTPUT);
  pinMode(binStoreEchoPin, INPUT);
  
  digitalWrite(binLidTriggerPin, LOW);
  digitalWrite(binStoreTriggerPin, LOW);
  
  // Verify connections and update Firebase
  Serial.println("\n=== Checking Sensor Connections ===");
  
  lidSensorWorking = verifyUltrasonicSensor(binLidTriggerPin, binLidEchoPin);
  Serial.println("Lid Sensor: " + String(lidSensorWorking ? "connected" : "not connected"));
  updateFirebase(LID_SENSOR_STATUS, lidSensorWorking ? "connected" : "not connected");
  
  storeSensorWorking = verifyUltrasonicSensor(binStoreTriggerPin, binStoreEchoPin);
  Serial.println("Store Sensor: " + String(storeSensorWorking ? "connected" : "not connected"));
  updateFirebase(STORE_SENSOR_STATUS, storeSensorWorking ? "connected" : "not connected");
  
  myservo.attach(servoPin);
  servoWorking = verifyServo();
  Serial.println("Servo: " + String(servoWorking ? "connected" : "not connected"));
  updateFirebase(SERVO_STATUS, servoWorking ? "connected" : "not connected");
  
  // Initialize bin status
  updateFirebase(BIN_POWER, "on");
  updateFirebase(LID_STATUS, "closed");
  updateFirebaseInt(BIN_FILLED, 0);
  
  if (!lidSensorWorking || !storeSensorWorking || !servoWorking) {
    Serial.println("\nWARNING: Some components are not connected!");
  } else {
    Serial.println("\nAll components are connected successfully!");
  }
  
  myservo.write(0);
}

void loop() {
  if (!lidSensorWorking && !storeSensorWorking && !servoWorking) {
    Serial.println("Critical failure: No components working");
    delay(5000);
    return;
  }

  // Check bin power status and height from Firebase
  checkBinPowerStatus();
  checkBinHeight();

  if (!binPowerStatus) {
    if (servoWorking) {
      myservo.write(0);  // Ensure lid is closed when powered off
    }
    delay(1000);  // Check power status every second
    return;
  }

  if (isMeasurementPaused) {
    if (millis() - measurementPauseTime >= 6000) {
      isMeasurementPaused = false;
      isServoMoving = false;
      if (servoWorking) {
        myservo.write(0);
        updateFirebase(LID_STATUS, "closed");
      }
    }
    return;
  }

  // Measure distances and update Firebase
  if (lidSensorWorking) {
    lidDistance = measureDistance(binLidTriggerPin, binLidEchoPin);
    if (lidDistance >= 0) {
      Serial.print("\nLid Distance: ");
      Serial.print(lidDistance);
      Serial.print(" cm");
    }
  }

  if (storeSensorWorking && !isServoMoving) {
    storeDistance = measureDistance(binStoreTriggerPin, binStoreEchoPin);
    if (storeDistance >= 0) {
      int fillPercentage = calculateFillPercentage(storeDistance);
      updateFirebaseInt(BIN_FILLED, fillPercentage);
      
      Serial.print(" | Bin Level: ");
      Serial.print(fillPercentage);
      Serial.print("%");
      
      if (fillPercentage >= 95) {
        Serial.print(" *** BIN FULL - ALERT! ***");
      }
    }
  }

  // Lid operation
  if (lidSensorWorking && servoWorking && lidDistance >= 0) {
    if (lidDistance < 5 && !isServoMoving) {
      isServoMoving = true;
      servoStartTime = millis();
      myservo.write(90);
      updateFirebase(LID_STATUS, "opening");
    }

    if (isServoMoving && (millis() - servoStartTime >= 500)) {
      measurementPauseTime = millis();
      isMeasurementPaused = true;
      myservo.write(180);
      updateFirebase(LID_STATUS, "open");
      delay(2000);
      myservo.write(0);
      updateFirebase(LID_STATUS, "closed");
      delay(2000);
    }
  }

  delay(100);
}
