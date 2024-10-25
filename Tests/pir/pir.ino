const int led = 13;              // LED positive terminal to digital pin 13
const int sensor = A0;           // Signal pin of sensor to analog pin A0
const int threshold = 500;       // Adjust this threshold value as needed
int state = LOW;
int val = 0;

void setup() {
  pinMode(led, OUTPUT);
  pinMode(sensor, INPUT);
  Serial.begin(9600);
}

void loop() {
  val = analogRead(sensor);
  Serial.print("Sensor value: ");
  Serial.println(val);

  if (val > threshold) {
    digitalWrite(led, HIGH);
    delay(500);

    if (state == LOW) {
      Serial.println("Motion detected");
      state = HIGH;
    }
  } 
  else {
    digitalWrite(led, LOW);
    delay(500);

    if (state == HIGH) {
      Serial.println("The action/motion has stopped");
      state = LOW;
    }
  }
}