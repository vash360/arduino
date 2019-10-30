//Mettre en D3
bool Ligne= false ;
void setup()
{
  pinMode( 3 , INPUT);
  Serial.begin(9600);
}

void loop()
{
  Ligne = digitalRead(3) ;
  Serial.print(Ligne);
  Serial.println();
}


