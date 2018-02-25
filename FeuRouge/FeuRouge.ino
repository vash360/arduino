void setup()
{
  pinMode( 7 , OUTPUT);
  pinMode( 9 , OUTPUT);
  pinMode( 12 , OUTPUT);
}

void loop()
{
  digitalWrite(7 , HIGH);
  delay( 300 );
  digitalWrite(7 , LOW);
  delay( 300 );
  digitalWrite(9 , HIGH);
  delay( 300 );
  digitalWrite(9 , LOW);
  delay( 300 );
  digitalWrite(12 , HIGH);
  delay( 300 );
  digitalWrite(12 , LOW);
  delay( 300 );
}


