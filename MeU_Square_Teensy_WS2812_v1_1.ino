//////////////////////////////////////////////////////////////////////////
//Filenames: MeU_Teensy_WS2812_v3.ino
//Authors: Robert Tu
//Date Created: June 5, 2014
//Notes:
/*
  PLEASE NOTE: This version of MeU Long uses the Teensy 3.1 microcontroller which
  is Arduino compatible. You will need to download the Teensyduino software
  first before being able to program MeU Long.
  
  You can download it here:
  http://www.pjrc.com/teensy/teensyduino.html
  
  Once Teensyduino is installed, the Teensy board option will become available
  under the Tools Menu in the Arduino IDE
  
  Also the Adafruit NeoPixel Library had to be modified in order to work with
  this version of MeU. Please be sure to install the correct version of
  the NeoPixel library that came with this version of MeU.
  
  This is the Arduino file that controls the MeU panel. It utilizes Adafruit's
  NeoPixel and GFX libraries to control the WS2812 LEDs.
  
  In the main loop the program waits for a serial message sent from a mobile
  device via bluetooth. The message is then parsed for specific commands. 
  
  The message protocol is as follows:
  
  "bFFFFFFmessage\n\r"
  
  The first character is the mode command and will determine what kind of
  information will be displayed on the LED panel. 
  
  The next six characters are colour values in hex (RGB notation). The colour 
  of the text is determined by this value.
  
  The rest of the following characters is the actual text to be displayed on 
  the LED panel. 
  
  The \n\r characters are for the serial read andparsing function to determine 
  the end of the message.
  
  For example if the Arduino receives the following command:
  "bFF00FFHello there how are you?\n\r"
  
  MeU will display "Hello there how are you?" in magenta colour.
  
  For bike safety functions:
  
  "e0\r"
  
  The first character is the mode command and will determine what kind of
  information will be displayed on the LED panel. "e" is bike
  message mode.
  
  The next single digit character is the bike signal to display
  
  0 = Right Turn
  1 = Left Turn
  2 = Stop Signal
  3 = Ride Signal
  
  The \r characters are for the serial read and parsing function to determine 
  the end of the message.
  


*/

//////////////////////////////////////////////////////////////////////////

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>


#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

//This is the pin that the LED panel is attached to on the Arduino
#define PIN 6
#define HEIGHT 16
#define WIDTH 16

// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

// Example for NeoPixel Shield.  In this application we'd like to use it
// as a 5x8 tall matrix, with the USB port positioned at the top of the
// Arduino.  When held that way, the first pixel is at the top right, and
// lines are arranged in columns, progressive order.  The shield uses
// 800 KHz (v2) pixels that expect GRB color data.

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(WIDTH, HEIGHT, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

unsigned long textTimer;
unsigned long turnTimer;
unsigned long flashTimer;
unsigned long flashTextTimer;

const unsigned int MAX_INPUT = HEIGHT*WIDTH+3;
const unsigned int MAX_MSG = HEIGHT*WIDTH+3;
const unsigned int MATRIX_LENGTH = HEIGHT*WIDTH;

//Buffer tables to hold RGB data
byte RedTable[MATRIX_LENGTH];
byte BlueTable[MATRIX_LENGTH];
byte GreenTable[MATRIX_LENGTH];

int ByteCount;
int TableCount = 0;
boolean ReadyToDraw = false;
boolean ImageLoaded = false;


//This value changes the speed of scrolling. The value is in milliseconds
int TextDelayValue = 50;
int turnDelayValue = 75;
int flashDelayValue = 50;
int flashTextDelayValue = 50;

//This variable will retain the message
String Message = "";
HardwareSerial Uart = HardwareSerial();


//Default colours
byte Red = 255;
byte Green = 255;
byte Blue = 255;

String input_line;

int DisplayMode;

boolean showBike;
boolean showLeftPanel;
boolean showStop;



//Set x at the right side of the matrix so text can scroll from right to left
int x = matrix.width();

void setup()
{
  
  //The bluetooth module's baud rate is set at 115200 so this needs to match
  Uart.begin(115200);
  Uart.setTimeout(100);
  Serial.begin(115200);
  
  //Initializes the matrix and clears the screen
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(20);
  matrix.setTextColor(0xFFFF);
  matrix.fillScreen(0);
  //draw a red pixel at 0,0 to indicate panel is on
  matrix.drawPixel(0,0,drawRGB24toRGB565(255,0,0));
  matrix.show();
  
  //This initiates the timer
  textTimer = millis();
  turnTimer = millis();
  flashTimer = millis();
  //reset brightness
  matrix.setBrightness(40);
}

void loop()
{
  
  //This section of code receives the incoming message via bluetooth and parses through
  //the parseData Function
  
  
  if (Uart.available () > 0) 
  {
 
    input_line = Uart.readString();
    parseData(input_line);
    
    
  } else {
    int passedTextTime;
    Uart.flush();
      // This section times the scrolling message depending on what value
    // TextDelayValue is set at.
    switch (DisplayMode) {
      //display text 
      case 1: 
    
        passedTextTime = millis() - textTimer;
        if (passedTextTime > TextDelayValue) {
          
          //This function is defined in DisplayText.ino
          ScrollText(Message);
          
          textTimer = millis();
        }
        
        break;
      
      //display bike signal
      case 2:
      
    
      
        if (Message == "0") {
          //flash right bike symbol for 10 times
          DisplayTurnSymbol(0);
        } else if (Message == "1") { // if left turn arrow, flash on patch 0
          DisplayTurnSymbol(1);
        } else if (Message == "2") { // if stop signal scroll stop across patch
          
          Red = 255;
          Blue = 120;
          Green = 100;
          FlashText("STOP");
            
        } else if (Message == "3") { // if riding mode then flash patch 0 then patch 1
          DisplayBikeSymbol();
        }
        
        break;   
        
    }
    
    // no case 3 because if we receive image data we don't want it to loop over and over again
  }
  // end of incoming data
  
  
  
}


/**************************************************************************/
/*!
    @brief  Converts a 24-bit RGB color to an equivalent 16-bit RGB565 value

    @param[in]  r
                8-bit red
    @param[in]  g
                8-bit green
    @param[in]  b
                8-bit blue

    @section Example

    @code 

    // Get 16-bit equivalent of 24-bit color
    uint16_t gray = drawRGB24toRGB565(0x33, 0x33, 0x33);

    @endcode
*/
/**************************************************************************/
uint16_t drawRGB24toRGB565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
}

// This function parses data
void parseData (String data)
{
  
  // parse the data for command
  // first character (position 0) is the Mode command
  // next 6 characters (position 1 to 6) is the colour
  // remaining characters (position 7 to n) is the actual text to display
  // last character(n+1) is the carriage return '\r' character to indicate end of the message. 
  
  //temporary buffer to hold the message
  
 
  char Mode = data.charAt(0);
  //Convert hex colour data to integers
  //Set the colours here
  
  
 
  if (Mode == 'b') {
    Red = SerialReadHexByte(data.charAt(1), data.charAt(2));
    Green = SerialReadHexByte(data.charAt(3), data.charAt(4));
    Blue = SerialReadHexByte(data.charAt(5), data.charAt(6));
    
    //Extract the text message to be displayed
    
    Message = data.substring(7, data.indexOf('\r')) + "  ";
    DisplayMode = 1;
    //Serial.println(Message);
     
  } else if (Mode == 'e') {
    DisplayMode = 2;
    Message = data.substring(1, data.indexOf('\r'));
  } else if (Mode == 'i') {
    DisplayMode = 3;
    matrix.fillScreen(0);
    matrix.show();
    
    //temporary bytes to store converted RGB data
    byte Red;
    byte Green;
    byte Blue;

    //loop through matrices and extract RGB data from 1536 character string
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        int loc = x + y*WIDTH;
    
        Red = SerialReadHexByte(data.charAt(1+loc*6), data.charAt(2+loc*6));
        Green = SerialReadHexByte(data.charAt(3+loc*6), data.charAt(4+loc*6));
        Blue = SerialReadHexByte(data.charAt(5+loc*6), data.charAt(6+loc*6));
        
        //after converting store into RGB tables     
        RedTable[loc] = Red;
        GreenTable[loc] = Green;
        BlueTable[loc] = Blue;
          
       
      }
    }
    
    //Once tables have stored values, load up into matrix buffer to display
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        int loc = x + y*HEIGHT; 
        matrix.drawPixel(x, y, drawRGB24toRGB565(RedTable[loc], GreenTable[loc], BlueTable[loc]));
      }
    }
   
    matrix.show();
      
    
    }

  
}

//Function to convert hex to integer
byte SerialReadHexDigit(byte c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } 
    
}

//Function to convert hex to integer
byte SerialReadHexByte(byte d, byte e)
{
    byte a = SerialReadHexDigit(d);
    byte b = SerialReadHexDigit(e);
   
    return (a*16) + b;
}

//This function actually scrolls the text on the LED screen
void ScrollText (String Data) {
  
  //Multiply the length by 6 so it scrolls long enough to display the entire message.
  int length = int(Data.length()*6);
  
  //clear the display
  matrix.fillScreen(0);
 
  //set the colour as received 
  matrix.setTextColor(drawRGB24toRGB565(Red, Green, Blue));
 
  //set the cursor to around mid screen
  matrix.setCursor(x, 3);
 
  matrix.print(Data);

  //actual scrolling calculation
  if(--x < -length) {
    x = matrix.width();
  }
  matrix.show();
 
}






