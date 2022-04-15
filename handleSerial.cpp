//
// handleSerial.cpp
//

#include "handleSerial.h"


String serialCommand;


void handleSerialMonitor() {

  // all possible commands are listed here
  
  if(Serial.available() > 0) {

    serialCommand = Serial.readStringUntil('\n');

    // Leerzeichen am Anfang und Ende entfernen
    serialCommand.trim();
    
    Serial.print(F("Received a serialCommand: "));
    Serial.println(serialCommand);

    // TTS TextTo Speech
    if(serialCommand.substring(0, 3) == "tts") {
      // TTS Struktur
      // ttsNachricht1_Nachricht2_Nachricht3_Nachricht4
      
      Serial.print(F("Received a TTS command: "));
      Serial.println(serialCommand.substring(3));

      splitString(serialCommand.substring(3));
    }
    
    // STDOUT FIFO
    if(serialCommand.substring(0, 6) == "stdout") {
      Serial.println(F("Received command to stdout FIFO: "));
      stdoutFifo();
    }

    // READ FIFO
    if(serialCommand.substring(0, 4) == "read") {
      Serial.println(F("Received command to read something out of FIFO: "));
      getFifo(1);
    }

    // DEBUG FIFO
    if(serialCommand.substring(0, 9) == "fifodebug") {
      if(fifoDebug == true) {
        fifoDebug = false;
        Serial.println(F("Turned off debug messages for FIFO"));
      } else {
        fifoDebug = true;
        Serial.println(F("Turned on debug messages for FIFO"));
      }
    }

    // SERIAL FLASH LIST
    // SerialFlash alle Dateien auf dem Flash ausgeben
    if(serialCommand.substring(0, 15) == "serialflashlist") {
      Serial.println(F("Received command to print a file listing of flash: "));
      serialFlashListing();
    }

    // SERIAL FLASH DELETE
    if(serialCommand.substring(0, 14) == "serialflashdel") {
      Serial.println(F("Received command to delete files from flash: "));
      serialFlashDelete();
    }

    // SET NEW ANIMATION
    if(serialCommand.substring(0, 4) == "anim") {
      Serial.print(F("Received command to change LED animation to: "));
      changeLEDAnimation(serialCommand.substring(5));
    }

    // PLAY SOUND FILE
    if(serialCommand.substring(0, 5) == "sound") {
      Serial.print(F("Received command to play some sound: "));
      parseSoundCommand(serialCommand.substring(6));
    }

    // CHANGE LED MATRIX BRIGHTNESS
    if(serialCommand.substring(0, 10) == "brightness") {
      Serial.print(F("Received command to change matrix brightness from "));
      changeBrightness(serialCommand.substring(11));
    }

    // CLEAR LED MATRIX
    if(serialCommand.substring(0, 5) == "clear") {
      Serial.println(F("Received command to clear matrix"));
      clearMatrix();
    }

    // CHANGE VOLUME
    if(serialCommand.substring(0, 6) == "volume") {
      Serial.println(F("Received command to change volume from "));
      changeVolume(serialCommand.substring(7));
    }

    // HELP 
    if(serialCommand.substring(0, 4) == "help") {
      Serial.println(F("HELP\n----"));
      Serial.println(F("Type one of the following commands into Serial Monitor:"));
      Serial.println(F("tts                       TextToSpeech followed by the word or sentence separeted by _. Example: ttsHello my friend_This is the second sentence to be spoken by Spencer"));
      Serial.println(F("stdout                    Prints the content of the FIFO into Serial Monitor"));
      Serial.println(F("fifodebug                 Enables debug messages of the FIFO to find some errors during programming"));
      Serial.println(F("serialflashlist           List all file names on flash of Spencer in Serial Monitor"));
      Serial.println(F("serialflashdel            Will delete the files recording1-4.mp3 and recording1-4.wav from flash if existant"));
      Serial.println(F("anim                      Send the name of an animation to play. Animation name must be separated by _. Example: anim_random0.gif"));
      Serial.println(F("sound                     Play some pregenerated MP3-file. Group and filename must be given, seperated by _. Example: sound_5_randomNoise0"));
      Serial.println(F("brightness                Change the brightness of the LED matrix. Brightness can be from 0 - 255, seperated by _. Example: brightness_150"));
      Serial.println(F("clear                     Clear the LED matrix and show nothing"));
      Serial.println(F("volume                    Change volume of Spencer. Volume can be from 0.1 to 3.9, separated by _. Example: volume_2.0"));
      Serial.println(F("------------------------------------------------------------------------------------------------------------------------------------------------------------------------"));
    }
  }
}



void serialFlashSpaces(int num) {
  // SerialFlash: taken from SerialFlash example; SerialFlash on PIN5 - see Spencer.h
  for (int i=0; i < num; i++) {
    Serial.print(' ');
  }
}


void serialFlashError(const char *message) {
  // SerialFlash: taken from SerialFlash example; SerialFlash on PIN5 - see Spencer.h
  while (1) {
    Serial.println(message);
    delay(2500);
  }
}


void serialFlashListing() {
  // SerialFlash: listing of files; SerialFlash on PIN5 - see Spencer.h
  if (!SerialFlash.begin(5)) {
    serialFlashError("Unable to access SPI Flash chip");
  }

  SerialFlash.opendir();
  while (1) {
    char filename[64];
    uint32_t filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
      Serial.print(F("  "));
      Serial.print(filename);
      serialFlashSpaces(20 - strlen(filename));
      Serial.print(F("  "));
      Serial.print(filesize);
      Serial.print(F(" bytes"));
      Serial.println();
    } else {
      break; // no more files
    }
  }
}


void serialFlashDelete() {
  // SerialFlash addition: delete recordings files; SerialFlash on PIN5 - see Spencer.h

  // files to delete if they exist
  const char* files[] = {
    "recording-1.mp3",
    "recording-2.mp3",
    "recording-3.mp3",
    "recording-4.mp3",
    "recording-1.wav",    // following files not added by CircuitMess, just for testing by me!
    "recording-2.wav",
    "recording-3.wav",
    "recording-4.wav"
  };
  
  
  if (!SerialFlash.begin(5)) {
    serialFlashError("Unable to access SPI Flash chip");
  }

  // run through all files
  for(int i = 0; i < (sizeof(files) / sizeof(files[0])); i++) {
    // check if file exist on flash
    if(SerialFlash.exists(files[i])) {
      // delete it
      if(SerialFlash.remove(files[i])) {
        Serial.print(files[i]);
        Serial.println(F(" was deleted successfully!"));
      } else {
        Serial.print(F("Delete of "));
        Serial.print(files[i]);
        Serial.println(F(" failed!"));
      }
    } else {
      // skip it with message
      Serial.print(files[i]);
      Serial.println(F(" dont exist on flash. File will be skipped!"));
    }
  }
}


void changeLEDAnimation(String animation) {
  // animation indicates which animation to play:
  // see Spencer firmware for mor information or use "serialflashlist" command for all possible GIFs
  // firmware can be found here: https://github.com/CircuitMess/Spencer-Firmware
  // note: the animation name starts always with "GIF-"!
  
  // (pre)define variables
  char anim[30] = "GIF-";
  char thisChar;
  
  // get length of String
  int strLength = animation.length();
  
  // run through String
  for(int i = 0; i < strLength; i++) {

    // we receive already an String containing just the animation name to play!
    // no worrys abaout _ or unnecessary chars

    // thisChar is actual char
    thisChar = animation.charAt(i);

    anim[i + 4] = thisChar;   // add actual char to anim[]
  }

  // write the complete animation name in console
  Serial.println(anim);

  // start animation with the generated name
  LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter(anim)), true);
}


void parseSoundCommand(String command) {
  // split the command by _ because we have 13 different sound groups
  // we will receive a command looking like this:
  //  5_randomNoise0

  char group[3] = "";
  char filename[40] = "";
  char thisChar;

  bool pmode = false;   // false indicates we are pasring group, true indicates we are parsing filename
  byte counter = 0;     // save the loop position when switching from group to filename

  // get length of String
  int strLength = command.length();
  
  // run through String
  for(int i = 0; i < strLength; i++) {

    // thisChar is actual char
    thisChar = command.charAt(i);

    if(pmode == false) {
      // always begin to parse group
      if(thisChar == '_') {     // change the mode to parse filename
        pmode = true;
        
        // save the actual loop position
        counter = i + 1;
      } else {
        group[i] = thisChar;
      }
    }
    if(pmode == true) {
      // parse filename
      filename[i - counter] = thisChar;
    }
  }

  Serial.print(F("group: "));
  Serial.print(group);
  Serial.print(F(" filename: "));
  Serial.println(filename);

  // call playSoundFile function
  playSoundFile(atoi(group), filename);
}


void playSoundFile(int group, char filename[]) {
  // group indicates which SampleGroup to use. See SampleStore.h for more details https://github.com/CircuitMess/Spencer-Library/blob/master/src/Audio/SampleStore.h
  // filenmae indicates which file of this group to play
  // you can also use "serialflashlist" command for all possible pregenarated audio files

  switch (group) {
    case 0:
      Playback.playMP3(SampleStore::load(SampleGroup::Months, filename));
      break;
    case 1:
      Playback.playMP3(SampleStore::load(SampleGroup::Numbers, filename));
      break;
    case 2:
      Playback.playMP3(SampleStore::load(SampleGroup::Jokes, filename));
      break;
    case 3:
      Playback.playMP3(SampleStore::load(SampleGroup::Generic, filename));
      break;
    case 4:
      Playback.playMP3(SampleStore::load(SampleGroup::Time, filename));
      break;
    case 5:
      Playback.playMP3(SampleStore::load(SampleGroup::Special, filename));
      break;
    case 6:
      Playback.playMP3(SampleStore::load(SampleGroup::Weather, filename));
      break;
    case 7:
      Playback.playMP3(SampleStore::load(SampleGroup::Weekdays, filename));
      break;
    case 8:
      Playback.playMP3(SampleStore::load(SampleGroup::Volume, filename));
      break;
    case 9:
      Playback.playMP3(SampleStore::load(SampleGroup::Brightness, filename));
      break;
    case 10:
      Playback.playMP3(SampleStore::load(SampleGroup::Levels, filename));
      break;
    case 11:
      Playback.playMP3(SampleStore::load(SampleGroup::Error, filename));
      break;
    case 12:
      Playback.playMP3(SampleStore::load(SampleGroup::Funpack, filename));
      break;
    default:
      Serial.println(F("Error playSoundFile(): unknown group number!"));
      break;
  }
}


void changeBrightness(String value) {
  // change the LED matrix brightness
  // we receive the raw brightness value as String

  int brightness = value.toInt();

  // limit the value
  if(brightness > 255) {
    brightness = 255;
  }
  if(brightness < 1) {
    brightness = 1;
  }

  // print the change into console
  Serial.print(LEDmatrix.getBrightness());
  Serial.print(F(" to "));
  Serial.println(brightness);

  // set the value
  LEDmatrix.setBrightness(brightness);
}


void clearMatrix() {
  // clear the LED matrix

  // first call a stop animation
  LEDmatrix.stopAnimation();
  
  // then clear matrix
  LEDmatrix.clear();
}



void changeVolume(String value) {
  // change volume of Spencer
  // we receive the raw volume value as String
  // Playback.h says we can change the volume between 0.0 and 4.0, but volume 4.0 did not work, maybe a bug -> we reduce the volume to 3.9; and yes, 3.9 ist VERY LOUD (even with this small speaker)
  
  float volume = value.toFloat();

  // limit the value
  if(volume > 3.9) {
    volume = 3.9;
  }
  if(volume < 0.1) {
    volume = 0.1;
  }

  // print the change into console
  Serial.print(Playback.getVolume());
  Serial.print(F(" to "));
  Serial.println(volume);

  // set the new value
  Playback.setVolume(volume);
}
