//
// Fifo.h
//

#include <Arduino.h>
#include <CircuitOS.h>
#include <Spencer.h>
#include <PreparedStatement.h>
#include <string.h>

#define SIZE_FIFO 9
#define SIZE_FIFO_COMMAND 150


extern char FIFO[SIZE_FIFO][SIZE_FIFO_COMMAND + 1];     // FIFO of 9 arrays, max. 150 chars (+ \0 - byte)
extern int fifoPointerIn, fifoPointerOut;
extern int fifoFreeSpace;
extern int fifoUsedSpace;
extern bool fifoDebug;   // print FIFO debug messages to Serial Monitor?


void stdoutFifo();

String getFifo(int nums);

void emptyFifo(int pos);

void textToSpeech();
