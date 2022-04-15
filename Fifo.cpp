//
// Fifo.cpp
//

// simple Fifo (first in, first out) to store up to 9 sentences of 150 chars to be spoken by Spencer

#include "Fifo.h";


char FIFO[SIZE_FIFO][SIZE_FIFO_COMMAND + 1];
int fifoPointerIn = 0, fifoPointerOut = 0;

int fifoFreeSpace = SIZE_FIFO;    // free space in FIFO
int fifoUsedSpace = 0;            // used spcae of FIFO
bool fifoDebug = false;   // print FIFO debug messages to Serial Monitor?



void stdoutFifo() {
  // print all content of FIFO in console

  Serial.println(F("---------------------------------------------"));

  Serial.print(F("act FIFO In pointer: "));
  Serial.println(fifoPointerIn);

  Serial.print(F("act FIFO Out pointer: "));
  Serial.println(fifoPointerOut);

  Serial.print(F("FIFO free space: "));
  Serial.println(fifoFreeSpace);

  Serial.print(F("FIFO used space: "));
  Serial.println(fifoUsedSpace);

  Serial.println(F("---------------------------------------------"));

  for(int i = 0; i < SIZE_FIFO; i++) {
    
    Serial.print(F("FIFO["));
    Serial.print(i);
    Serial.println(F("]: "));
    
    for(int j = 0; j < SIZE_FIFO_COMMAND; j++) {
      if (isAlphaNumeric(FIFO[i][j]) || (isWhitespace(FIFO[i][j]))) {
        Serial.print(FIFO[i][j]);
      }
    }
    Serial.println();
    Serial.println(F("---------------------------------------------"));
  }
}



String getFifo(int nums) {
  // return current FIFO content, delets it and set the pointer to next position
  // 'nums' used to print multiple entrys one after another
  char buf[151];

  if(nums == 0 || !nums) {
    nums = 1;
  }

  // there must be something in fifo
  if(fifoUsedSpace > 0) {
    for(int i = 0; i < nums; i++) {
      
      // actual pointer to print
      int pointer = fifoPointerOut + i;

      // print in console
      for(int j = 0; j < SIZE_FIFO_COMMAND; j++) {
        if(fifoDebug) {
          Serial.print(FIFO[pointer][j]);
        }
        buf[j] = FIFO[pointer][j];
      }
      if(fifoDebug) {
        Serial.println();
      }

      // delete content
      emptyFifo(pointer);

      // FIFO Out Pointer +1
      if(fifoPointerOut == SIZE_FIFO - 1) {
        fifoPointerOut = 0;       // pointer to 0 if there is something else inside FIFO and we read something from last entry (restart FIFO)
      } else {
        // only increase Pointer Out if the supplied nums was done completely
        if(i == (nums - 1)) {
          fifoPointerOut++;
        }
      }
    }

    return String(buf);
    
  } else {
    Serial.println(F("Error getFifo: FIFO is empty."));
  }
}



void emptyFifo(int pos) {
  // deletes entry on given position

  if(!pos) {
    pos = 0;
  }
  
  for(int i = 0; i < SIZE_FIFO_COMMAND; i++) {
    FIFO[pos][i] = (char)0;
  }
  //FIFO[pos] = 0;

  // FIFO Used Space - 1
  fifoUsedSpace--;
  // FIFO Free Space + 1
  fifoFreeSpace++;
}



void splitString(String ttsString) {

  int j = 0;
  char thisChar;
  
  // remove spaces of String
  ttsString.trim();

  // get length of String
  int strLength = ttsString.length();
  if(fifoDebug) {
    Serial.print("Length of TTS command: ");
    Serial.println(strLength);
  }
  
  // run through String
  for(int i = 0; i < strLength; i++) {

    // is there some free space in FIFO?
    if(fifoUsedSpace < SIZE_FIFO) {
    
      // thisChar is actual char
      thisChar = ttsString.charAt(i);

      // write into FIFO
      if(j < SIZE_FIFO_COMMAND) {
        // only save chars, numbers and spaces
        if (isAlphaNumeric(thisChar) || isWhitespace(thisChar)) {
          FIFO[fifoPointerIn][j] = thisChar;
        }
      } else {
        Serial.println(F("Error splitString: TTS Command for FIFO too long! Processing command was cancled!"));
      }
      j++;

      // if there was an '_' found or the whole end of TTS command was reached
      if((thisChar == '_') || (i == strLength - 1)) {

        // internal counter j to 0;
        j = 0;
        // FIFO In Pointer +1
        if(fifoPointerIn == SIZE_FIFO - 1) {
          fifoPointerIn = 0;          // Pointer In to 0 if there is free space and last entry was an last position (restart FIFO)
        } else {
          fifoPointerIn++;
        }
        
        // FIFO Used Space +1
        fifoUsedSpace++;
        // FIFO Free Space -1
        fifoFreeSpace--;

        if(fifoDebug) {
          Serial.println(F("End of TTS command reached! -> Switch to next one"));
        }
      }
    } else {
      Serial.println(F("Error splitString: FIFO is full!"));
    }
  }

  // after whole String was parsed start with Spencer talking
  textToSpeech();
}
