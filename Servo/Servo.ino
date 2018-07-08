#include <Servo.h>
#include <IRremote2.h>

int Angle = 0 ;
int Left = 0 ;
int Right = 0 ;
int Step = 0 ;
Servo servo_pin_5;
int IRCode = 0 ;
//libraries at http://duinoedu.com/dl/lib/grove/EDU_IRremote_GroveDupont/
IRrecv2 monRecepteur_pin11(11);

void setup()
{
  servo_pin_5.attach(5);
  monRecepteur_pin11.enableIRIn(); 
  Serial.begin(9600);
  Angle = 90 ;

  Left = 16720605 ;

  Right = 16761405 ;

  Step = 10 ;

  servo_pin_5.write( Angle );

}

void loop()
{
  IRCode = monRecepteur_pin11.lireCodeIr()  ;
  Serial.print("Ir =");
  Serial.print(" ");
  Serial.print(IRCode);
  Serial.print(" ");
  Serial.println();
  if (( ( IRCode ) == ( Left ) ))
  {
    Angle = ( Angle - Step ) ;
  }
  if (( ( IRCode ) == ( Right ) ))
  {
    Angle = ( Angle + Step ) ;
  }
  if (( ( Angle ) < ( 0 ) ))
  {
    Angle = 0 ;
  }
  if (( ( Angle ) > ( 180 ) ))
  {
    Angle = 180 ;
  }
  servo_pin_5.write( Angle );
}


