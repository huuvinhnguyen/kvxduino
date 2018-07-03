
int pinOut = 10;

void setup() {

  Serial.begin(9600);
  pinMode(10, OUTPUT);

}

void loop() {
  
  digitalWrite(pinOut, HIGH);
  delay(5000);
   digitalWrite(pinOut, LOW);
   delay(5000);


}
