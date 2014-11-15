//////////////////////////////////////////////////////////////////////////
//Filenames: TimerEvents.ino
//Authors: Robert Tu
//Date Created: June 5, 2014
//Notes:
/*
  This sub module contains all the Timing functions.

*/

//////////////////////////////////////////////////////////////////////////

void FlashEvent() {
  int passedTime = millis() - flashTimer;
  if (passedTime > flashDelayValue) {
    if (showLeftPanel == true) {
   
      showLeftPanel = false;
    } else {
      
      
      showLeftPanel = true;
    }
    flashTimer = millis();
  }
  
}

void TurnEvent() {
  int passedTime = millis() - turnTimer;
  if (passedTime > turnDelayValue) {
    if (showBike == true) {
   
      showBike = false;
    } else {
      
      
      showBike = true;
    }
    turnTimer = millis();
  }
  
}

//Used for flashing the word Stop
void FlashTextEvent() {
  int passedTime = millis() - flashTextTimer;
  if (passedTime > flashTextDelayValue) {
    if (showStop == true) {
      showStop = false;
    } else {
      showStop = true;
    }
    flashTextTimer = millis();
  }
}
