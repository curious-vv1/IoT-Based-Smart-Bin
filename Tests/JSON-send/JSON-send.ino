#include "secrets.h"
#include <Firebase.h>
#include <ArduinoJson.h>

/* Use the following instance for Test Mode (No Authentication) */
Firebase fb(REFERENCE_URL);

int binHeight;   // Variable to store the bin height input from the user
int binFilled;   // Variable to store the bin filled percentage
bool updateFlag = false; // Flag to indicate when a new value has been entered

void setup() {
  Serial.begin(115200);

  #if !defined(ARDUINO_UNOWIFIR4)
    WiFi.mode(WIFI_STA);
  #else
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
  #endif
  WiFi.disconnect();
  delay(1000);

  /* Connect to WiFi */
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
  Serial.println();

  #if defined(ARDUINO_UNOWIFIR4)
    digitalWrite(LED_BUILTIN, HIGH);
  #endif

  /* ----- */

  /* Get bin height from user through Serial */
  Serial.println("Enter bin height: ");
  while (Serial.available() == 0) {
    // Wait for user input
  }
  binHeight = Serial.parseInt();
  Serial.print("Bin height entered: ");
  Serial.println(binHeight);

  /* Get initial bin filled percentage */
  Serial.println("Enter initial bin filled percentage: ");
  while (Serial.available() == 0) {
    // Wait for user input
  }
  binFilled = Serial.parseInt();
  Serial.print("Initial bin filled percentage: ");
  Serial.println(binFilled);

  // Call a function to update Firebase
  updateFirebase();
}

void loop() {
  // Check if user has entered new input via the Serial Monitor
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Read the entire input line

    // Check if input is not empty and is a valid number
    if (input.length() > 0 && input.toInt() != 0) {
      binFilled = input.toInt();  // Parse the input for binFilled
      Serial.print("New bin filled percentage: ");
      Serial.println(binFilled);

      // Set the flag to true indicating that we need to update Firebase
      updateFlag = true;
    } else {
      Serial.println("Invalid input. Please enter a valid number.");
    }
  }

  // Only update Firebase if there is new data to be processed
  if (updateFlag) {
    updateFirebase();    // Update Firebase with the new binFilled value
    updateFlag = false;  // Reset the flag after updating
  }
}

/* Function to update the Firebase with current smart bin data */
void updateFirebase() {
  /* ----- Serialization: Send Smart Bin data to Firebase ----- */
  // Create a JSON document to hold the bin data
  StaticJsonDocument<256> docOutput;

  // Add smart bin data to the JSON document
  docOutput["binFilled"] = binFilled;  // Update binFilled with the latest value
  docOutput["binLidSensor"] = "connected";
  docOutput["binStoreSensor"] = "connected";
  docOutput["lid"] = "closed";
  docOutput["servo"] = "connected";
  docOutput["binHeight"] = binHeight;  // Store user input for bin height
  docOutput["switch"] = "on";

  // Create a string to hold the serialized JSON data
  String output;

  // Serialize the JSON document to a string
  serializeJson(docOutput, output);

  // Send the serialized JSON data to Firebase
  fb.setJson("bin1", output);

  Serial.println("Smart bin data updated on Firebase.");
}
