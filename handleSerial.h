//
// handleSerial.h
//


#include <SerialFlash.h>

#include "Fifo.h";



void handleSerialMonitor();

void splitString(String ttsString);

void serialFlashListing();

void serialFlashDelete();

void serialFlashSpaces(int num);

void serialFlashError(const char *message);

void changeLEDAnimation(String animation);

void parseSoundCommand(String command);

void playSoundFile(int group, char filename[40]);

void changeBrightness(String value);

void clearMatrix();

void changeVolume(String value);
