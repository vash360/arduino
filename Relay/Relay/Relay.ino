//Author David Lanier from Vivre a St Hilaire 
//dlanier@free.fr

//realy is plugged in digital pin 7
const int relayPin =7;

void setup() {
  // Set Pin for relay as an output
  pinMode(relayPin, OUTPUT);
}

void loop() {
  digitalWrite(relayPin, HIGH); //Switch On
  delay(5000);
  digitalWrite(relayPin, LOW); //Switch Off
  delay(5000);
}
