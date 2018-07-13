
int pinOut = 13;

void setup() {

  Serial.begin(9600);
  pinMode(13, OUTPUT);

}

void loop() {
  
  digitalWrite(pinOut, HIGH);
  delay(5000);
   digitalWrite(pinOut, LOW);
   delay(5000);

}
