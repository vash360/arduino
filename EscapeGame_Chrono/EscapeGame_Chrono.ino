
/* David Lanier - dlanier@free.fr
 *  Escape game count down chronometer 
*/

#include <LedControl.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>  //Software Serial Port 
//#include <BK3254.h>

#define RxD_BT 9      //Pin 9 for RX for Bluetooth
#define TxD_BT 10     //Pin 10 for TX for Bluetooth
#define RxD_BK3254 2      //Pin 2 for RX for BK3254 MP3 player
#define TxD_BK3254 3     //Pin 3 for TX for BK3254 MP3 player

#define PUSHBUTTONPIN 7   // Pushbutton connected to pin 7

//Bluetoooh recepter HC06
SoftwareSerial BTSerie(RxD_BT,TxD_BT); 

//BK3254 MP3 Player
/*SoftwareSerial BKSerial(RxD_BK3254, TxD_BK3254);
#define resetBTpin 5
BK3254 BT(&BKSerial, resetBTpin);
#define INITVOLUME "10"
*/

//display screen : Using 4 MAX 7219 connected as a dot matrix LED screen
//static const int din = 13, clk = 11, cs = 12; //pins for the 4 MAX 7219
//static const int devices = 4;//Number of MAX7219 connected together
LedControl lc = LedControl(13, 11, 12, 4);

//Globals for the chronometer
static const bool bBlinkAtStartup = false; //do we want the chronometer to blink at startup ?
static bool bFinished             = false;//Is countdown finished ?
static bool bStarted              = false;//Has countdown started ?
static bool bPaused               = false;//Is countdown paused ?
static bool bBlink                = bBlinkAtStartup; //To blink before starting or when finished
static bool bShowTime             = true; //Used in blink mode to show/hide the display to produce a blink

static const int BlinkDelay         = 400; //Delay in milliseconds for blinking
static unsigned long BlinkStartTime = 0;//Time stored to toggle when we show/hide when blinking

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

//The display on the screen is going to be made with 4 digits : m1 m2 : s1 s2, with m1 being the first character of the minutes, m2 the second char of the minutes, same apply to s1 and s2 for the seconds.
static char m1 = '0', m2 = '0', s1 = '0', s2 = '0'; //current display time of the game

//INITIAL TIME OF THE GAME
static int GameDurationInMinutes = 30;      

static unsigned long  TimeElapsedBeforePause   = 0;//When entering in pause mode, we store the time elapsed in that variable
static unsigned long  StartTime                = 0;//Is the time where we start the count down or unpause the game 

#define eeAddress 0 //Location in EEPROM to store game duration to retrieve it when powering on again the Arduino

//For drawing numbers on the LED screen
const byte colonChar PROGMEM = B00100100;//To display a colon character to separate minutes and seconds

//Array to display the numbers as 8x5 dots matrix LED, set it in flash memory instead of SRAM to save SRAM (using the PROGMEM keyword)
const byte numberArray[313] PROGMEM =
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

//Gets the location of the character in the numberArray
int GetAddressInArray(char c) {   
  int location = 0;
  if (c >= '0' && c <= '9'){
    location = 5 * (c - '0'); //5 because we have 5 columns for each character and (c - '0') is the value of the character c, say c='6', (c - '0') will be the integer 6
  }
  
  return location;
}

//Draw a character on screen
void DrawCharacter(int DisplayNumber, int startAddr)
{
  for (int i = 0; i < 5; i++)
    lc.setColumn(DisplayNumber, 1 + i, pgm_read_byte(&numberArray[startAddr + i])); //1 + i because we skip the first column to center more the character in a single MAX 7219
}

void DrawSemiColumnChar()
{
  lc.setColumn(2, 7, pgm_read_byte(&colonChar));//Draw on 2nd MAX 7219 at column 7 the semi colon character as a single column
}

void DisplayTime() { //displays the time on the 4 MAX7219
  /*Serial.print("m1: ");
  Serial.println((char)m1);
  Serial.print("m2: ");
  Serial.println((char)m2);
  Serial.print("s1: ");
  Serial.println((char)s1);
  Serial.print("s2: ");
  Serial.println((char)s2);
  */
  
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
  StartTime = millis();
  bStarted  = true;
  bFinished = false;
  bPaused   = false;
  bBlink    = false;
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
  }else
  {
    outc1 = '5';
  }

  //(outc1 - '0') gives an integer value (in the 0, 9 range) of the outc1 character
  //(inTwoDigitsNumber - (10 * (outc1 - '0'))) gives a value between 0 and 9
  //adding '0' gives the matching char for this value, so a value of 4 gives the char '4'
  outc2 = (inTwoDigitsNumber - (10 * (outc1 - '0'))) + '0';
}

//Convert minutes and seconds into 4 characters to be displayed on screen
void SetTimeCharacters(int minutes, int seconds)
{
  GetCharactersFromNumber(minutes, m1, m2);//Put result in globals m1 and m2
  GetCharactersFromNumber(seconds, s1, s2);//Put result in globals s1 and s2
}

void setup() {
  //Init the 4 Max 7219 screen
  for (int i = 0 ; i < 4/*devices */; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 0);
    lc.clearDisplay(i)  ;
  }
  delay(500); 
  
  //Init serial for print debugging
  Serial.begin(115200);

  // Bluetooth 
  pinMode(RxD_BT, INPUT); 
  pinMode(TxD_BT, OUTPUT); 
  BTSerie.begin(9600);
  delay(500); 

  //Push button
  pinMode(PUSHBUTTONPIN, INPUT);

  //Check if we have a gameduration stored in EEPROM
  int oldGameDuration = 0;
  EEPROM.get(eeAddress, oldGameDuration);
  if (oldGameDuration > 0 && oldGameDuration < 59){
      GameDurationInMinutes = oldGameDuration;
  }

  //Music
  /*BT.begin();
  uint16_t time=millis();
  const uint16_t timeout=500;
  while (BT.volumeSet(INITVOLUME) != 1 && time - millis() < timeout);
  BT.musicModeRepeatOne();
  BT.switchInputToCard();
  const int numSongs = BT.cardUsbGetSongsCount();
  Serial.print("numSongs : ");
  Serial.println(numSongs);
  BT.musicTogglePlayPause();
 BT.getCurrentInput();
  switch (BT.MusicState) {
    case (BT.Playing):
      //Serial.println(F("Playing music"));
      if (BT.InputSelected == BT.SD || BT.InputSelected == BT.USB) {
        BT.cardUsbGetSongsCount(); //get number of song on card or USB and currently played song:
        if (BT.InputSelected == BT.SD) BT.cardGetCurrentPlayingSongNumber();
        if (BT.InputSelected == BT.USB) BT.usbGetCurrentPlayingSongNumber();
        Serial.print("Playing song "); Serial.print(BT.CurrentlyPlayingSong); Serial.print(" of "); Serial.print(BT.NumberOfSongs); Serial.println(".");
      }
      break;
    case (BT.Idle):
      Serial.println(F("Music stoped"));
      break;
  }
  */
}

void ClearLCD()
{
  for (int i = 0 ; i < 4/*devices*/ ; i++) {
    lc.clearDisplay(i)  ;
  }
}

void CheckBluetoothCommands()
{
  if (BTSerie.available()) { 
    char recvChar = BTSerie.read(); 
    Serial.print(recvChar); 
    if (recvChar == 'S')//Start
    {
      if (false == bFinished && false == bStarted){
        StartCountDown();//Start the count down
      }
    }else
    if (recvChar == 'P')//Pause
    {
      TogglePause();
    }else
    if (recvChar == 'R')//Reset
    {
      Reset();
    }
    else
    if (recvChar == '+')//Add 5 minutes
    {
      ChangeMinutes(5);
    }
    else
    if (recvChar == '-')//Remove 5 minutes
    {
      ChangeMinutes(-5);
    }
    if (recvChar == 'M')//Add 1 minute
    {
      ChangeMinutes(1);
    }
    else
    if (recvChar == 'L')//Remove 1 minute
    {
      ChangeMinutes(-1);
    }
  } 
}

//Change the game duration
void ChangeMinutes(int diff)
{
  if (bFinished){
    return;
  }
  if (!bStarted || bPaused){
    //Change time only when we haven't started or we are in pause mode
    GameDurationInMinutes += diff;
    ///Clamp value
    if (GameDurationInMinutes < 1){
      GameDurationInMinutes = 1;
    }else
    if (GameDurationInMinutes > 59){
      GameDurationInMinutes = 59;
    }

    //Store the game duration in EEPROM so we find it when we restart the Arduino
    EEPROM.put( eeAddress, GameDurationInMinutes);  
  }
}

void Reset()
{
  bFinished                = false;
  bStarted                 = false;
  bPaused                  = false;
  bBlink                   = bBlinkAtStartup;
  BlinkStartTime           = millis();
  TimeElapsedBeforePause   = 0; 
  StartTime                = 0;
}

//When entering the Pause mode
void Pause()
{
  TimeElapsedBeforePause += millis() - StartTime; //This is the time we already spent, store it and use += to add it in case we pause several times
}

//When exiting the Pause mode
void UnPause()
{
  StartTime = millis();//Update StartTime so that we continue conting the time from now
}

void TogglePause(){
  if (bFinished || false == bStarted){
      return;
  }
  
  bPaused = !bPaused;
  bBlink  = bPaused; //We blink during the pause
  //BT.musicTogglePlayPause();//toggle pause for music
  if (bPaused){
    BlinkStartTime = millis();
    Pause();
  }else{
    UnPause();
  }
}

void CheckForPausePushButton()
{
    if (digitalRead(PUSHBUTTONPIN)== HIGH  && bStarted) {         // check if the input is HIGH (button released)
      TogglePause();
      delay(500);//Wait a bit to avoid a toggle/untoggle
    }
}

//Main loop
void loop(){

  CheckBluetoothCommands();

  Serial.print("Paused : ");
  Serial.println(bPaused);
  Serial.print("Finished : ");
  Serial.println(bFinished);
  Serial.print("Started : ");
  Serial.println(bStarted);

  CheckForPausePushButton();

  Serial.print("bBlink : ");
  Serial.println(bBlink);

  //Are we in blink mode (when paused or game duration elapsed)
  if (bBlink){
    if (millis() - BlinkStartTime > BlinkDelay){
      BlinkStartTime = millis();
      bShowTime = !bShowTime; //toggle the show/hide of the current display time
    }

    Serial.print("bShowTime : ");
    Serial.println(bShowTime);
  
    if (false == bShowTime){
      ClearLCD();
      return;
    }
  }

  if (bFinished){
    DisplayTime();
    return;
  }
  
  Serial.print("GameDurationInMinutes : ");
  Serial.println(GameDurationInMinutes);

  //Get elapsed time
  unsigned long remainingTimeInSeconds  = GameDurationInMinutes*SECS_PER_MIN*1000;
  if (bStarted){ //Has the count down started ?
    if (bPaused){//Are we paused ?
      remainingTimeInSeconds -= TimeElapsedBeforePause; //We are un pause mode
    }else{
      remainingTimeInSeconds -= millis() - StartTime + TimeElapsedBeforePause; //We have started the count down and are not paused
    }
  }
  remainingTimeInSeconds /= 1000; //Convert in seconds
  
  Serial.print("remainingTimeInSeconds : ");
  Serial.println(remainingTimeInSeconds);
  const int minutes = numberOfMinutes(remainingTimeInSeconds); //get number of minutes (betweeen 0 and 59)
  const int seconds = numberOfSeconds(remainingTimeInSeconds); //get number of seconds (betweeen 0 and 59)
  Serial.print("minutes : ");
  Serial.println(minutes);
  Serial.print("seconds : ");
  Serial.println(seconds);
  SetTimeCharacters(minutes, seconds); //convert minutes and seconds in characters
  DisplayTime(); //display them on the LCD
  if (m1 == '0'& m2 == '0' & s1 == '0' & s2 == '0') {
    bFinished               = true;
    TimeElapsedBeforePause  = 0;
    bBlink                  = true; //blink when finished
  }
}
