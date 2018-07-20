int in = 4; 
int out = 8;  
int state = HIGH;  
int r;           
int currentState = LOW;    
long time = 0;       
long debounce = 200;
void setup()
{
    Serial.begin(9600);

  pinMode(4, INPUT);
  pinMode(8, OUTPUT);
}
void loop()                    
{
 
  r = digitalRead(4);

  if (r != currentState && millis() - time > debounce) {
     Serial.println("pressed");

    if (state == HIGH)
      state = LOW;
    else 
      state = HIGH;
    time = millis();   
    delay(300); 
  }
  digitalWrite(8, state);
  currentState = r;
}
