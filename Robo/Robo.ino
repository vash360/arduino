
//David Lanier - dlanier@free.fr

#include <Servo.h>
#include <SoftwareSerial.h>  //Software Serial Port 

//Using software serial for Blue tooth reading values , use Android Blue tooh serial controller application to send chars using a repeatable delay of 60ms
SoftwareSerial BTSerial(6,7);

String readString = "";

//Distance Sensor SR04
#define trigPin 4
#define echoPin 3

//The servo takes a value between 0 and 179 for its speed, 
//The servos need some intermediate values to go from their minimum values to their maximum
//You need to calibrate your servos to check what the values are (there is a CalibrateServos function in this code where you can send through Serial a value to test)
const int ZeroServoSpeed         = 95;
const int IntermediateServoSpeed = 100;
const int SlowForwardSpeed       = 98;
const int SlowBackwardSpeed      = 92;
const int FastForwardSpeed       = 135;
const int FastBackwardSpeed      = 0;
const int delayTime              = 15;

Servo leftWheel, rightWheel;

//Module HC06 Arduino PIN : 1234
//On Android use the application "Bluetooth Serial Controller" with a repeatable delay of 60 ms

void setup()
{
  //Use Serial for the Bluetooth HC06
  Serial.begin(9600);
  BTSerial.begin(9600);
  
  //Servo motors
  rightWheel.attach(9);//Servo motor plugged on D9
  leftWheel.attach(10);//Servo motor plugged on D10
  Stop();

  //Distance sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void ReadBlueTooth()
{
  if (BTSerial.available()) { 
    char recvChar = BTSerial.read(); 
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
    if (recvChar == 'S')
    {
      Stop();
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

void LeftEngine(int leftSpeed)
{
  Serial.print(" leftWheel = ");
  Serial.println(leftSpeed);
  
  leftWheel.write (leftSpeed);
}

void RightEngine(int rightSpeed)
{
  Serial.print(" rightWheel = ");
  Serial.println(rightSpeed);
  
  rightWheel.write (rightSpeed);
}

void IntermediateLeft()
{
  LeftEngine(IntermediateServoSpeed);
  delay( 2 );
}

void IntermediateRight()
{
  RightEngine(IntermediateServoSpeed);
  delay( 2 );
}

void MoveForwardSlow()
{
  IntermediateLeft();
  SetServos(SlowForwardSpeed, SlowBackwardSpeed);
}

void MoveForwardFast()
{
  IntermediateLeft();
  SetServos(FastForwardSpeed, FastBackwardSpeed);
}

void MoveBackwardSlow()
{
  IntermediateRight();
  SetServos(SlowBackwardSpeed, SlowForwardSpeed);
}

void MoveBackwardFast()
{
  IntermediateRight();
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
  IntermediateLeft();
  IntermediateRight();
  SetServos(SlowForwardSpeed, SlowForwardSpeed);
}

void TurnRightFast()
{
  IntermediateLeft();
  IntermediateRight();
  SetServos(FastForwardSpeed, FastForwardSpeed);
}

void CalibrateServos()
{
  //Read number sent from Serial
   while (Serial.available()) {
    char c = Serial.read();  //gets one byte from serial buffer
    readString += c; //makes the String readString
    delay(2);  //slow looping to allow buffer to fill with next character
  }

 //Convert it, constrain it and send it to the servos
 if (readString.length() >0) {
    Serial.println(readString);  //so you can see the captured String
    int n = readString.toInt();  //convert readString into a number
    n = constrain(n, -255, 255);//
    Serial.println(n); //so you can see the integer
    SetServos(n, n);
    readString="";
  } 
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

void ReadDistance()
{
   long duration, distance;
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  Serial.print("Distance : ");
  Serial.println(distance);
}

void loop()
{
  //CalibrateServos();
    //MoveForwardFast();
  ReadBlueTooth();
  ReadDistance();
  //Test();
}
