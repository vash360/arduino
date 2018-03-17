//Author David Lanier from Vivre a St Hilaire 
//dlanier@free.fr

/*
 * We are using the Arduino shield OneSheeld
 * //https://1sheeld.com/

OPTIONAL:
To reduce the library compiled size and limit its memory usage, you
can specify which shields you want to include in your sketch by
defining CUSTOM_SETTINGS and the shields respective INCLUDE_ define. 
*/

//Don't forget to set the serial switch on the OneSheeld to SW to upload and set it to HW to run on the shield

#define CUSTOM_SETTINGS
#define INCLUDE_SMS_SHIELD
#define INCLUDE_TERMINAL_SHIELD
//Calling for Arduino Voice Recognize Shield
#define INCLUDE_VOICE_RECOGNIZER_SHIELD

/* Include 1Sheeld library. */
#include <OneSheeld.h>

//Set commands to be recognized by the Arduino Voice Recognition Shield
const char on[] = "go";
const char off[] = "stop";

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

  digitalWrite(relayPin, HIGH); //Switch Off
}

void loop() 
{
  //Check if 1Sheeld's Arduino Voice Recognition Shield received a new command
  if(VoiceRecognition.isNewCommandReceived())
  {
    //Compare the last command received by the Arduino Voice Recognition Shield with the command "on"
    if(!strcmp(on,VoiceRecognition.getLastCommand()))
    {
      //Then turn the relay on
     digitalWrite(relayPin, LOW); //Switch On
    }
    //Compare the last command received by the Arduino Voice Recognition Shield with the command "off"
    else if(!strcmp(off,VoiceRecognition.getLastCommand()))
    {
      //Then turn the relay off
     digitalWrite(relayPin, HIGH); //Switch Off
    }
  }
}

/* This function will be invoked each time a new sms comes to the smartphone. 
 *  You need to have the oneSheeld application installed on your smartphone and connectedto the Arduino shield
*/
void SMSReceivedFunction (String phoneNumber ,String messageBody)
{
  Terminal.println(phoneNumber);
  Terminal.println(messageBody);

  if (messageBody.equalsIgnoreCase(String("on"))){
      digitalWrite(relayPin, LOW); //Switch On
  }
  else
  if (messageBody.equalsIgnoreCase(String("off"))){
      digitalWrite(relayPin, HIGH); //Switch Off
  }
}

