//https://techtutorialsx.com/2016/12/23/esp8266-detecting-rain-drops/
const byte interruptPin = 13;
volatile boolean checkInterrupt = false;
int numberOfInterrupts = 0;
 
unsigned long debounceTime = 1000;
unsigned long lastDebounce=0;
 
void setup() {
 
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
 
}
 
void handleInterrupt() {
  checkInterrupt= true;
}
 
void loop() {
 
  if(checkInterrupt == true && ( (millis() - lastDebounce)  > debounceTime )){
 
      lastDebounce = millis();
      checkInterrupt = false;
      numberOfInterrupts++;
 
      Serial.print("Rain detected ");
      Serial.println(numberOfInterrupts); 
 
  }else checkInterrupt = false;
 
}
