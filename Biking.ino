//////////////////////////////////////////////////////////////////////////
//Filenames: Biking.ino
//Authors: Robert Tu
//Date Created: June 5, 2014
//Notes:
/*
  This sub module contains all the Biking functions.
  
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

void DisplayTurnSymbol (int SymbolIndex) {
  
  TurnEvent();
  if (showBike == true) {
    matrix.fillScreen(0);
    switch (SymbolIndex) {
      case 0:// right turn signal
         //right turn
        matrix.fillTriangle(28, 0, 31, 3, 28, 6, drawRGB24toRGB565(255, 175, 0));
        matrix.fillRect(21, 2, 7, 3, drawRGB24toRGB565(255, 175, 0));
        
        break;
      
      case 1://left turn signal
        //left turn
        
        matrix.fillTriangle(0, 3, 3, 0, 3, 6, drawRGB24toRGB565(255, 175, 0));
        matrix.fillRect(4, 2, 7, 3, drawRGB24toRGB565(255, 175, 0));
        
        break;
    }
  } else {
    matrix.fillScreen(0);
  }
    
  matrix.show();  
}

void DisplayBikeSymbol() {
  FlashEvent();
  matrix.fillScreen(0);
  if (showLeftPanel == true) {
    //left side
    matrix.fillRect(0, 1, 14, 5, drawRGB24toRGB565(255, 100, 100));
  } else {
    //right side
    matrix.fillRect(17, 1, 14, 5, drawRGB24toRGB565(255, 100, 100));
  }
  matrix.show();
   
}

void FlashText (String Data) {
  
  FlashTextEvent();
  
  matrix.fillScreen(0);
  matrix.setTextColor(drawRGB24toRGB565(Red, Green, Blue));
  matrix.setCursor(6, 0);
  if (showStop == true) {
    
    matrix.fillScreen(0);   
      
  } else {
    matrix.print(Data);

  }
  
  matrix.show();
 
}
