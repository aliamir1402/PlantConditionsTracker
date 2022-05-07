const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034

long duration;
float distanceCm;
float distanceMeter;

void setup() {
  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = (duration) * SOUND_SPEED/2;
  
  // Convert to inches
  distanceMeter = distanceCm / 1000;
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.print(distanceCm);Serial.println("cm");
  //Serial.print("Distance (m): ");
  //Serial.print(distanceMeter);Serial.println("m");
  
  delay(1000);
}
