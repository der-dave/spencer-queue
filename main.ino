#include <Arduino.h>
#include <CircuitOS.h>
#include <Spencer.h>
#include <PreparedStatement.h>
#include <string.h>


// custom functions/library
#include "Fifo.h";
#include "handleSerial.h";



// Spencer definitions ------------------------------------------------------
PreparedStatement* statement = nullptr;
bool synthesizing = false;

const char* listenResult;
IntentResult* intentResult;
bool processingIntent = false;
bool detectingIntent = false;

// speech to text (STT) server address and certificate
const char* sttUrl = "http://xxx.xxx.xxx.xxx:12101/api/speech-to-intent";    // custom address used for STT
const char* sttCert = "false";                                            // "" -> use default CircuitMess certificate; "false" -> dont use a certificate; "xx:xx:xx ..." -> use custom certificate

// text to speech (TTS) server address and certificate
const char* ttsUrl = "http://xxx.xxx.xxx.xxx:5500/api/tts?voice=nanotts%3Ade-DE";    // custom address used for TTS
const char* ttsCert = "false";                                                    // "" -> use default CircuitMess certificate; "false" -> dont use a certificate; "xx:xx:xx ..." -> use custom certificate
// END Spencer definitions --------------------------------------------------



void BTN_press(){
  if (Net.getState() == WL_CONNECTED) {
    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-talk.gif")), true);
    if(detectingIntent){
      Serial.println("Another listen and intent detection operation is already pending");
    }else{
      detectingIntent = true;
      listenResult = nullptr;
      Recording.addJob({ &listenResult });
    }
  } else {
    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-noWifi.gif")), true);
    Playback.playMP3(SampleStore::load(SampleGroup::Error, "wifi"));
  }
}


void speechPlay(TTSError error, CompositeAudioFileSource* source){
  synthesizing = false;
  if(error != TTSError::OK){
    Serial.printf("Text to speech error %d: %s\n", error, TTSStrings[(int) error]);
    delete source;
    delete statement;
    statement = nullptr;
    return;
  }
  Playback.playWAV(source);                     // VERY IMPORTANT: use playWAV() instead of playMP3() because our server is sending an uncompressed WAV file
  Playback.setPlaybackDoneCallback([](){
    // we can do something here after the speech was finished
  });
  delete statement;
  statement = nullptr;
}


void listenProcess(){
  LEDmatrix.stopAnimation();
  LEDmatrix.clear();
}


void listenError(){
}


void listenCheck(){
  if(listenResult != nullptr && !processingIntent){
    processingIntent = true;
    delete intentResult;
    intentResult = nullptr;
    SpeechToIntent.addJob({ listenResult, &intentResult, sttUrl, sttCert });
    //SpeechToIntent.addJob({ listenResult, &intentResult });

    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-loading7.gif")), true);

  }
  if(processingIntent && intentResult != nullptr){
    detectingIntent = false;
    processingIntent = false;
    listenResult = nullptr;
    if(intentResult->error != IntentResult::Error::OK && intentResult->error != IntentResult::Error::INTENT){
      Serial.printf("Speech to text error %d: %s\n", intentResult->error, STIStrings[(int) intentResult->error]);
      listenError();
      delete intentResult;
      intentResult = nullptr;
      return;
    }
    if(intentResult->intent == nullptr){
      intentResult->intent = (char*) malloc(5);
      memcpy(intentResult->intent, "NONE", 5);
    }
    if(intentResult->transcript == nullptr){
      intentResult->transcript = (char*) malloc(1);
      memset(intentResult->transcript, 0, 1);
    }
    listenProcess();
    delete intentResult;
    intentResult = nullptr;
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Spencer.begin();
  Spencer.loadSettings();
  Input::getInstance()->setBtnPressCallback(BTN_PIN, BTN_press);

  Net.set("SSID", "PASSWORD");
  Net.connect([](wl_status_t state){
    if (Net.getState() == WL_CONNECTED) {
      Playback.playMP3(SampleStore::load(SampleGroup::Special, "startup"));
    }
  });
}

void loop() {
  LoopManager::loop();
  listenCheck();

  handleSerialMonitor();

  // handle FIFO as long as there is something inside and sysnthesizing not active!
  if((fifoUsedSpace > 0) && (synthesizing == false)) {
    textToSpeech();
  }
}



// additional functions for Spencer ------------------------------------------
void textToSpeech() {
  if(synthesizing) {
    Serial.println("Another speech synthesis operation is already pending");
  } else {
    synthesizing = true;
    delete statement;
    statement = new PreparedStatement();
    statement->addTTS(getFifo(1), ttsUrl, ttsCert);
    statement->prepareWAV(speechPlay);
  }
}
// END additional functions for Spencer --------------------------------------
