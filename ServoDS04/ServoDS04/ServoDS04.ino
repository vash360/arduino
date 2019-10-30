
//David Lanier - dlanier@free.fr

//****************************************************************************************************************
//**  Each time you modify this file and upload it to Arduino, unplug pin 0 and 1 RX and TX or it won't upload. **
//****************************************************************************************************************

#include <Servo.h>

Servo servo1, servo2;

//Module HC06 Arduino PIN : 1234
//On Android use the application "Bluetooth Serial Controller" with a repeatable delay of 60 ms

void setup()
{
  //Use Serial for the Bluetooth HC05
  Serial.begin(9600);

  //Servo motors
  servo1.attach(9);//Servo motor plugged on D9
  servo2.attach(11);//Servo motor plugged on D9
}
void loop()
{
    //enter the value to set in the servo in the Serial open at 9600
  while (Serial.available() > 0){
    int c = Serial.parseInt();
    Serial.println(c);
    servo1.write(c);
    servo2.write(c);
  }
}

