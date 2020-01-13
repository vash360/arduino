
/* David Lanier - dlanier@free.fr
 *  Escape game count down chronometer 
 *  With MP3 player support BK3254
 *  Display screen for remaiing time are 4 MAX7219 Dot LED matrices connected
*/
#include <LedControl.h> //For MAX7219 control download from https://github.com/wayoda/LedControl
#include <EEPROM.h> //To store data that survive a power off from Arduino
#include <SoftwareSerial.h>  //SoftwareSerial Port

#define RxD_BT 9      //Pin 9 for RX for Bluetooth
#define TxD_BT 10     //Pin 10 for TX for Bluetooth
#define RxD_BK3254 2  //RX for BK3254 MP3 player
#define TxD_BK3254 3  //TX for BK3254 MP3 player

#define PUSHBUTTON_PAUSE_PIN 7   // Pushbutton connected to pin 7 for pause
#define PUSHBUTTON_START_PIN 8   // Pushbutton connected to pin 7 for start/restart

//We are using 2 SoftwareSerial and not both can listen (receive commands) at the same time, so we will have to use listen() to switch to one or the other to receive commands
//But usually the bluetooth BTSerial will be the listener, we will switch to listening on the BKSerial when we want to send a command and get the status as a result
//Bluetoooh recepter HC06
SoftwareSerial BTSerial(RxD_BT,TxD_BT); 

//BK3254 MP3 Player, we will use the SD Card to play music
SoftwareSerial BKSerial(RxD_BK3254, TxD_BK3254);
//UART Commands for BK3254 can be found from : https://github.com/tomaskovacik/BK3254/wiki/Supported-commands-and-event-send-from-module
#define SD_CARD_MODE_CMD       "COM+MSD\r\n"
#define TOGGLE_PLAY_PAUSE_CMD  "COM+PP\r\n"
#define NEXT_TRACK_CMD         "COM+PN\r\n"
#define PREV_TRACK_CMD         "COM+PV\r\n"
#define REPEAT_ONE_CMD         "COM+MPM1\r\n"
#define MAX_VOLUME_CMD         "COM+VOLF\r\n" //Maximum volume is 15 but must be set in Hexadecimal hence the : "F" after "VOL"
#define AUTOPLAY_OFF_CMD       "COM+MP3AUTOPLYOFF\r\n"
#define SD_CARD_PAUSED         "SD_PU"

//Display screen : Using 4 MAX 7219 connected as a dot matrix LED screen
//see https://www.instructables.com/id/16x8-LED-dot-matrix-with-MAX7219-module/
static const int din = 13, clk = 11, cs = 12; //pins for the 4 MAX 7219
static const int devices = 4;//Number of MAX7219 connected together
LedControl lc = LedControl(din, clk, cs, devices);

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
//complete alphabet can be found on http://www.gammon.com.au/forum/?id=11516
const byte numberArray[] PROGMEM =
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
  for (int i = 0; i < 5; i++){
    lc.setColumn(DisplayNumber, 1 + i, pgm_read_byte(&numberArray[startAddr + i])); //1 + i because we skip the first column to center more the character in a single MAX 7219  
  }
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

  PlayMusic();
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

void ResetMusic()
{
  SendCommandAndGetResultFromBK3254(PREV_TRACK_CMD); //Reset to current song, by setting previous song and next song which is current
  SendCommandAndGetResultFromBK3254(NEXT_TRACK_CMD);
  PauseMusic();
}

String SendCommandAndGetResultFromBK3254(char* command)
{
  //UART Commands for BK3254 can be found from : https://github.com/tomaskovacik/BK3254/wiki/Supported-commands-and-event-send-from-module
  String result; 
  BKSerial.listen(); //Usually the bluetooth is lstening, so we have to tell SoftwareSerial to listen on this port for BK3254 to be able to read the reply from the sent command
  BKSerial.write(command);//Send command
  delay(200);//waiting a bit, it is required to get a correct reply (if you don't do this, you only get OK but not always the full reply)
  while (BKSerial.available() > 0) { 
    char ch = BKSerial.read(); //read reply char by char
    //Serial.print("Receive from BKSerial : ");
    //Serial.println(ch);
    result += ch; //concat the char into a string result
  }
  BTSerial.listen();//Put back listening for SoftwareSerial for bluetooth
  Serial.print("Result from command : ");
  Serial.print(command);
  Serial.print(" is : ");
  Serial.println(result);
  return result;
}

//String parsing we parse resultFromSerial to find the substring searchString
//The resultFromSerial usually contains several substrings separated by "\n"
bool SearchStringForResult(String resultFromSerial, char* searchString)
{
  Serial.print("Searching string : ");
  Serial.print(resultFromSerial);
  Serial.print(" for substring : ");
  Serial.println(searchString);
  
  // Tokenize
   char *tmpbuf = strtok(resultFromSerial.c_str(), "\n");
   while(tmpbuf != NULL) {
      String line = tmpbuf;
      if (line == String(searchString)){
        Serial.println("result is true");
        return true;
      }
      Serial.println((char *)tmpbuf);
      tmpbuf = strtok(NULL, "\n");
   }
   Serial.println("result is false");
  return false;
}

bool IsSDCardPlaused(String resultFromSerial)
{
  //Usually the string is a kind of "OK\nSD_PA" or "OK\nSD_PU"
  //SD_PU meaning SD card is in pause mode and SD_PA means SD card is playing music
  return SearchStringForResult(resultFromSerial, SD_CARD_PAUSED);
}

void PauseMusic()
{
  String res = SendCommandAndGetResultFromBK3254(TOGGLE_PLAY_PAUSE_CMD);
  if (false == IsSDCardPlaused(res)){
    BKSerial.write(TOGGLE_PLAY_PAUSE_CMD);//Set it in pause mode as it is not yet paused
  }
}

void PlayMusic()
{
  String res = SendCommandAndGetResultFromBK3254(TOGGLE_PLAY_PAUSE_CMD);
  if (IsSDCardPlaused(res)){
    BKSerial.write(TOGGLE_PLAY_PAUSE_CMD);//Set it in play mode as it is paused
  }
}

//Called once when the Arduino is powered on
void setup() {
  //Init the 4 Max 7219 screens
  for (int i = 0 ; i < devices; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 4);
    lc.clearDisplay(i);
  }
  
  //Init serial for print debugging
  Serial.begin(115200);

  // Bluetooth controller to receive commands from a smartphone/tablet
  pinMode(RxD_BT, INPUT); 
  pinMode(TxD_BT, OUTPUT); 
  BTSerial.begin(9600);
  delay(500); 

  //Push button setup
  pinMode(PUSHBUTTON_PAUSE_PIN, INPUT);
  pinMode(PUSHBUTTON_START_PIN, INPUT);

  //Check if we have a GameDurationInMinutes value stored in EEPROM 
  //This is when you modify the initial GameDurationInMinutes and don't want to update it each time you power on the Arduino
  int oldGameDuration = 0;
  EEPROM.get(eeAddress, oldGameDuration);
  if (oldGameDuration > 0 && oldGameDuration < 59){
      GameDurationInMinutes = oldGameDuration;
  }

  //Music setup BK3254 module
  delay(500);
  pinMode(RxD_BK3254, INPUT); 
  pinMode(TxD_BK3254, OUTPUT); 
  BKSerial.begin(9600);//We will communicate with it by sending UART commands
  SendCommandAndGetResultFromBK3254(SD_CARD_MODE_CMD); //Set SD Card reading mode
  SendCommandAndGetResultFromBK3254(REPEAT_ONE_CMD); //Set repeat 1 to repeat the same song
  SendCommandAndGetResultFromBK3254(AUTOPLAY_OFF_CMD); //Set autoplay when started off
  SendCommandAndGetResultFromBK3254(MAX_VOLUME_CMD);//Set volume at maximum level
  PauseMusic();
  
  //Set SoftwareSerial listening on Bluetooh as the BKSerial SoftwareSerial is used to send commands only
  //And SoftwareSerial can only listen on one port
  BTSerial.listen();
}

void ClearLCD()
{
  for (int i = 0 ; i < devices ; i++) {
    lc.clearDisplay(i)  ;
  }
}

void CheckBluetoothCommands()
{
   //check if the Bluetooth controller has received commands from a smartphone/tablet
  if (BTSerial.available()) { 
    char recvChar = BTSerial.read(); //Read a single char
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
    //Change time only when we haven't started yet or have reset or we are in pause mode
    GameDurationInMinutes += diff;
    ///Clamp value
    if (GameDurationInMinutes < 1){
      GameDurationInMinutes = 1;
    }else
    if (GameDurationInMinutes > 59){
      GameDurationInMinutes = 59;
    }

    //Store the game duration in EEPROM so we retrieve it when we restart the Arduino
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
  ResetMusic();
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

void ToggleStartRestart(){
  
  if (bFinished){
    Reset();
    return;
  }

  if (false == bStarted){
    //Start
    StartCountDown();
    return;
  }

  //We have started and are not yet finished
  Reset();
}

void TogglePause(){
  if (bFinished || false == bStarted){
      return;
  }
  
  bPaused = !bPaused;
  bBlink  = bPaused; //We blink during the pause mode
  if (bPaused){
    PauseMusic();
    BlinkStartTime = millis();
    Pause();
  }else{
    PlayMusic();
    UnPause();
  }
}

void CheckForPushButtons()
{
    if (digitalRead(PUSHBUTTON_PAUSE_PIN)== HIGH  && bStarted) {         // check if the input is HIGH (button released)
      TogglePause();
      delay(500);//Wait a bit to avoid a toggle/untoggle
    }

    if (digitalRead(PUSHBUTTON_START_PIN)== HIGH) {         // check if the input is HIGH (button released)
      ToggleStartRestart();
      delay(500);//Wait a bit to avoid a toggle/untoggle
    }
}

//Main loop
void loop(){

  CheckBluetoothCommands();

  /*Serial.print("Paused : ");
  Serial.println(bPaused);
  Serial.print("Finished : ");
  Serial.println(bFinished);
  Serial.print("Started : ");
  Serial.println(bStarted);
*/

  CheckForPushButtons();

  //Serial.print("bBlink : ");
  //Serial.println(bBlink);

  //Are we in blink mode (when paused or game duration elapsed)
  if (bBlink){
    if (millis() - BlinkStartTime > BlinkDelay){
      BlinkStartTime = millis();
      bShowTime = !bShowTime; //toggle the show/hide of the current display time
    }

    //Serial.print("bShowTime : ");
    //Serial.println(bShowTime);
  
    if (false == bShowTime){
      ClearLCD();
      return;
    }
  }

  if (bFinished){
    DisplayTime();
    return;
  }
  
  //Serial.print("GameDurationInMinutes : ");
  //Serial.println(GameDurationInMinutes);

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
  
  //Serial.print("remainingTimeInSeconds : ");
  //Serial.println(remainingTimeInSeconds);
  const int minutes = numberOfMinutes(remainingTimeInSeconds); //get number of minutes (betweeen 0 and 59)
  const int seconds = numberOfSeconds(remainingTimeInSeconds); //get number of seconds (betweeen 0 and 59)
  //Serial.print("minutes : ");
  //Serial.println(minutes);
  //Serial.print("seconds : ");
  //Serial.println(seconds);
  SetTimeCharacters(minutes, seconds); //convert minutes and seconds in characters
  DisplayTime(); //display them on the LCD
  if (m1 == '0'& m2 == '0' & s1 == '0' & s2 == '0') {
    bFinished               = true;
    TimeElapsedBeforePause  = 0;
    bBlink                  = true; //blink when finished
    PauseMusic();
  }
}
