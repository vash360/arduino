//Author David Lanier from Vivre a St Hilaire 
//dlanier@free.fr

/*
 * We are using the Arduino shield OneSheeld
 * 
OPTIONAL:
To reduce the library compiled size and limit its memory usage, you
can specify which shields you want to include in your sketch by
defining CUSTOM_SETTINGS and the shields respective INCLUDE_ define. 
*/

//don't forget to use serial switch to SW to upload and set it to HW to run on the shield

#define CUSTOM_SETTINGS
#define INCLUDE_SMS_SHIELD
#define INCLUDE_TERMINAL_SHIELD

/* Include 1Sheeld library. */
#include <OneSheeld.h>


//relay is plugged in digital pin 7
const int relayPin =7;

void setup() 
{
  // Set Pin for relay as an output
  pinMode(relayPin, OUTPUT);
  
  /* Start communication. */
  OneSheeld.begin();
  SMS.setOnSmsReceive(&SMSReceivedFunction);
  Terminal.println("Init");
}

void loop() 
{
}

/* This function will be invoked each time a new sms comes to smartphone. */
void SMSReceivedFunction (String phoneNumber ,String messageBody)
{
  Terminal.println(phoneNumber);
  Terminal.println(messageBody);

  if (messageBody== String("On")){
      digitalWrite(relayPin, HIGH); //Switch On
  }
  else
  if (messageBody== String("Off")){
      digitalWrite(relayPin, LOW); //Switch Off
  }
}

