int triggerPin=4;
int echoPin=7;

float distance;
float duration;

void setup()
{
  Serial.begin(9600);
  pinMode(triggerPin,OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop()
{
 
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(10); // Wait for 2 microseconds
  
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10); // Wait for 10 microseconds
  
  digitalWrite(triggerPin, LOW);
  
  
  duration=pulseIn(echoPin, HIGH);
  distance=(duration*0.034)/2;
  
  Serial.print("\n Distance: ");
  Serial.print(distance);
  Serial.print(" cm");
  delay(100);
}
              