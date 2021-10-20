/*
 * LED Matrix 
 * 
 * Version 2
 * 
 */

#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>
#include <WiFi.h>
#include <WiFiSettings.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <PNGdec.h> 
#include <AsyncHTTPRequest_Generic.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "globaldefs.h"
#include "pixelart.h"
#include "LEDMatrix.h"
#include "RotEncs.h"
#include "WifiConfigPortal.h"
#include "audio_reactive.h"
#include "SpectrumAnalyzer.h"
#include "DisplayFiles.h"
#include "webpages.h"
#include "WebServiceConfig.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  setupRotEncs();
  setupSpectrumAnalyzer();  
  setupLEDMatrix();
  setupWifiConfigPortal();
  setupWebserver();
}

void loop() {
  // put your main code here, to run repeatedly:
  switch(mode){
  case 0:
    readFiles();
    break;
  case 1:
    showAudio();
    break;
  case 2:
    serialDebug();
    printArray(leddefault, 1000);
    break;
  }
}
