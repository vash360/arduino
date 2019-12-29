
/* David Lanier - dlanier@free.fr
 *  Escape game count down
*/

#include <LedControl.h>
#include <EEPROM.h>

//Using 4 MAX 7219 connected as a display dot matrix LED screen
static const int din = 13, clk = 11, cs = 12; //pins for the 4 MAX 7219
static const int devices = 4;//Number of MAX7219 connected together
LedControl lc = LedControl(din, clk, cs, devices);

bool finished = false;//Is countdown finished ?

#define USE_SERIAL Serial
#define eeAddress 0 //Address for the Arduino's EEPROM (internal non volatile memory)

// macros from DateTime.h
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  

//The display is going to be made with 4 digits, m1 m2 : s1 s2 m1m2 being the number of minutes and s1s2 being the number of seconds displayed
char m1 = '0', m2 = '0', s1 = '0', s2 = '0'; //current display time of the game

static unsigned long  TimerStart = 30*SECS_PER_MIN*1000;//In milliseconds, this is the time we have when the game starts, modify this to add/reove some time

const byte semiColonChar = B00100100;//To display a semi colon character to separate minutes and seconds

//Array to display the numbers as 8x5 dots matrix LED
const byte numberArray[313] =
{
  B01111100, B10100010, B10010010, B10001010, B01111100, /*columns for 0*/
  B00100010, B01000010, B11111110, B00000010, B00000010, /*columns for 1*/
  B01000110, B10001010, B10010010, B10010010, B01110010, /*columns for 2*/
  B10000100, B10010010, B10110010, B11010010, B10001100, /*columns for 3*/
  B00011000, B00101000, B01001000, B11111110, B00001000, /*columns for 4*/
  B11110100, B10010010, B10010010, B10010010, B10001100, /*columns for 5*/
  B01111100, B10010010, B10010010, B10010010, B01001100, /*columns for 6*/
  B10000000, B10001110, B10010000, B10100000, B11000000, /*columns for 7*/
  B01101100, B10010010, B10010010, B10010010, B01101100, /*columns for 8*/
  B01100100, B10010010, B10010010, B10010010, B01111100  /*columns for 9*/
};

int GetAddressInArray(char c) {   //gets the location of the character in the numberArray
  int location = 0;
  if (c >= '0' && c <= '9'){
    location = 5 * (c - '0'); //5 because we have 5 columns for each character and (c - '0') is the value of the character c, say c='6', (c - '0') will be the integer 6
  }
  
  return location;
}

//Draw a character on screen
void DrawCharacter(int column, int startAddr)
{
  for (int i = 0; i < 5; i++)
    lc.setColumn(column, 1 + i,numberArray[startAddr + i]); //1 + i because we skip the first column to center more the character in a single MAX 7219
}

void DrawSemiColumnChar()
{
  lc.setColumn(2, 7, semiColonChar);//Draw on 2nd MAX 7219 at column 7 the semi colon character as a single column
}

void DisplayTime() { //displays the time on the 4 MAX7219
  USE_SERIAL.print("m1: ");
  USE_SERIAL.println((char)m1);
  USE_SERIAL.print("m2: ");
  USE_SERIAL.println((char)m2);
  USE_SERIAL.print("s1: ");
  USE_SERIAL.println((char)s1);
  USE_SERIAL.print("s2: ");
  USE_SERIAL.println((char)s2);
  
  const int startAddress_m1 = GetAddressInArray(m1);
  const int startAddress_m2 = GetAddressInArray(m2);
  const int startAddress_s1 = GetAddressInArray(s1);
  const int startAddress_s2 = GetAddressInArray(s2);

//Draw the 4 characters to display the remaining time
  DrawCharacter(3, startAddress_m1);
  DrawCharacter(2, startAddress_m2);
  DrawCharacter(1, startAddress_s1); //1 is the second MAX 7219 starting from right to left
  DrawCharacter(0, startAddress_s2); //0 is the top most right MAX 7219
  DrawSemiColumnChar();
}

void StartCountDown()
{
  unsigned long curTime = millis();
  EEPROM.put( eeAddress, curTime);  //Store the current time in EEPROM
  finished              = false;
}
//Get the 2 digits number into 2 characters, such as inTwoDigitsNumber = 34, outc1 will be '3' and outc2 will be '4'
void GetCharactersFromNumber(byte inTwoDigitsNumber, char& outc1, char& outc2)
{
   if (inTwoDigitsNumber < 10) {
    outc1 = '0';
  } else if (inTwoDigitsNumber < 20) {
    outc1 = '1';
  } else if (inTwoDigitsNumber < 30) {
    outc1 = '2';
  } else if (inTwoDigitsNumber < 40) {
    outc1 = '3';
  } else if (inTwoDigitsNumber < 50) {
    outc1 = '4';
  } else
  {
    outc1 = '5';
  }

  //(outc1 - '0') gives an integer value (in the 0, 9 range) of the outc1 character
  //(inTwoDigitsNumber - (10 * (outc1 - '0'))) gives a value between 0 and 9
  //adding '0' gives the matching char for this value, so a value of 4 gives the char '4'
  outc2 = (inTwoDigitsNumber - (10 * (outc1 - '0'))) + '0';
}

//Convert minutes an seconds into 4 characters to be displayed on screen
void SetTimeCharacters(int minutes, int seconds)
{
  GetCharactersFromNumber(minutes, m1, m2);//Put result in globals m1 and m2
  GetCharactersFromNumber(seconds, s1, s2);//Put result in globals s1 and s2
}

void setup() {
  for (int i = 0 ; i < devices ; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 0);
    lc.clearDisplay(i)  ;
  }
  USE_SERIAL.begin(115200);
  USE_SERIAL.println();
  USE_SERIAL.print("Devices : ");
  USE_SERIAL.println(devices);
  USE_SERIAL.println();

  StartCountDown();//Start the count down
}

//Main loop
void loop(){
  if (finished){
    return;//The count down is finished, nothing to do until we restart
  }
  
  //Get elapsed time
  unsigned long oldTime = 0;
  //millis() is the current time in milliseconds, EEPROM.get(eeAddress, oldTime) is the time at which we started the countdown and our TimerStart is the number of minutes and seconds we want to count
  const unsigned long remainingTimeInSeconds  = (TimerStart - (millis() - EEPROM.get(eeAddress, oldTime)) ) / 1000;
  USE_SERIAL.print("remainingTimeInSeconds : ");
  USE_SERIAL.println(remainingTimeInSeconds);
  const int minutes = numberOfMinutes(remainingTimeInSeconds);
  const int seconds = numberOfSeconds(remainingTimeInSeconds);
  USE_SERIAL.print("minutes : ");
  USE_SERIAL.println(minutes);
  USE_SERIAL.print("seconds : ");
  USE_SERIAL.println(seconds);
  SetTimeCharacters(minutes, seconds);
  DisplayTime();
  if (m1 == '0'& m2 == '0' & s1 == '0' & s2 == '0') {
    finished = true;
  }
}
