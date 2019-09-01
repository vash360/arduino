
//David Lanier - dlanier@free.fr

//****************************************************************************************************************
//**  Each time you modify this file and upload it to Arduino, unplug pin 0 and 1 RX and TX or it won't upload. **
//****************************************************************************************************************

#include <Servo.h>
#include <SoftwareSerial.h>  //Software Serial Port 

//Is the value which makes the servo motor not rotate.
//The servo takes a value between 0 and 179 for its speed, values of 95 makes it still and values < 95 turn in one way values > 95 turn the other way, but the range 
//for maximium speed is about 5 after/before the zero
const int ZeroServoSpeed      = 95;
const int SlowForwardSpeed    = 98;
const int SlowBackwardSpeed   = 92;
const int FastForwardSpeed    = 179;
const int FastBackwardSpeed   = 0;
const int delayTime           = 15;
Servo leftWheel, rightWheel;

//Module HC06 Arduino PIN : 1234
//On Android use the application "Bluetooth Serial Controller" with a repeatable delay of 60 ms

void setup()
{
  //Use Serial for the Bluetooth HC06
  Serial.begin(9600);

  //Servo motors
  rightWheel.attach(9);//Servo motor plugged on D9
  leftWheel.attach(10);//Servo motor plugged on D10
  Stop();
}

void ReadBlueTooth()
{
  if (Serial.available()) { 
    char recvChar = Serial.read(); 
    Serial.print("Bluetooth character received :"); 
    Serial.println(recvChar); 

    //SLOW
    if (recvChar == 'f')
    {
      MoveForwardSlow();
    }
    else
    if (recvChar == 'b')
    {
      MoveBackwardSlow();
    }
    else
    if (recvChar == 'l')
    {
      TurnLeftSlow();
    }
    else
    if (recvChar == 'r')
    {
      TurnRightSlow();
    }
    else
    //FAST
    if (recvChar == 'F')
    {
      MoveForwardFast();
    }
    else
    if (recvChar == 'B')
    {
      MoveBackwardFast();
    }
    else
    if (recvChar == 'L')
    {
      TurnLeftFast();
    }
    else
    if (recvChar == 'R')
    {
      TurnRightFast();
    }
  }else{
     //No bluetooth commands available
     Stop();
  }
}
void Stop()
{
  SetServos(ZeroServoSpeed, ZeroServoSpeed);
}

void SetServos(int leftSpeed, int rightspeed)
{
  Serial.print("rightWheel = ");
  Serial.print(rightspeed);
  Serial.print(" leftWheel = ");
  Serial.println(leftSpeed);
  
  rightWheel.write(rightspeed);
  leftWheel.write (leftSpeed);
  //delay(delayTime);
}

void MoveForwardSlow()
{
  SetServos(SlowForwardSpeed, SlowBackwardSpeed);
}

void MoveForwardFast()
{
  SetServos(FastForwardSpeed, FastBackwardSpeed);
}

void MoveBackwardSlow()
{
  SetServos(SlowBackwardSpeed, SlowForwardSpeed);
}

void MoveBackwardFast()
{
  SetServos(FastBackwardSpeed, FastForwardSpeed);
}

void TurnLeftSlow()
{
  SetServos(SlowBackwardSpeed, SlowBackwardSpeed);
}

void TurnLeftFast()
{
  SetServos(FastBackwardSpeed, FastBackwardSpeed);
}

void TurnRightSlow()
{
  SetServos(SlowForwardSpeed, SlowForwardSpeed);
}

void TurnRightFast()
{
  SetServos(FastForwardSpeed, FastForwardSpeed);
}

void Test()
{
  for (int i = 1;i<20;i++){ 
    MoveForwardSlow();
  }
  for (int i = 1;i<20;i++){ 
    MoveForwardFast();
  }
  for (int i = 1;i<20;i++){ 
    MoveBackwardSlow();
  }
  for (int i = 1;i<20;i++){ 
    MoveBackwardFast();
  }
  for (int i = 1;i<20;i++){ 
    TurnLeftSlow();
  }
  for (int i = 1;i<20;i++){ 
    TurnLeftFast();
  }
  for (int i = 1;i<20;i++){ 
    TurnRightSlow();
  }
  for (int i = 1;i<20;i++){ 
    TurnRightFast();
  }
  Stop();
}
void loop()
{
  ReadBlueTooth();
}
