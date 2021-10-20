// global variables
#define FIRMWARE_VERSION "v2.0.0"
int mode = 0;         // modus operandi: 0 = Slide Show, 1 = Audio Spectrum Analyser, 2 = Debug Info
int maxmode = 3;      // number of modes
int pattern = 0;      // Spectrum Analyzer Pattern (0-5)
int maxpatterns = 6;  // number of patterns
int brightness = 63;  // LED Panel Brightness
int gain = 12;     // Microphone Input Gain
uint8_t numBands;     // FFT
uint8_t barWidth;     // FFT
uint16_t displayTime;
bool autoChangePatterns = false;

// debouncing buttons
unsigned long prevMillis = 0; // save last time of interrupt
int debounceTime = 150;       // time to wait until button press is valid

// defining rotary encoders pins
#define ROTENC_0_CLK  23
#define ROTENC_0_DT   22
#define ROTENC_0_SW   21
#define ROTENC_1_CLK  33
#define ROTENC_1_DT   25
#define ROTENC_1_SW   34

// defining LEDMatrix
#define DATA_PIN 2
const int M_HEIGHT = 16;  // height of LED Matrix
const int M_WIDTH = 16;   // width of LED Matrix
cLEDMatrix<M_WIDTH, M_HEIGHT, HORIZONTAL_ZIGZAG_MATRIX> leds;
cLEDText ScrollingMsg;

// Wifi yes/no
bool nowifi = false;

// Functions to access a file on the SD card
File myfile;

// Display PNGs on LED Matrix
uint32_t ledpic[M_WIDTH*M_HEIGHT];  // storing picture information for LED panel (16x16, 24bit per pixel)
PNG png; // Cpt. Obvious :)

// Web Service
const String default_httpuser = "admin";
const String default_httppassword = "admin";
const int default_webserverporthttp = 80;
const String modedesc0 = "Slide Show";
const String modedesc1 = "Sound Spectrum Analyzer";
// configuration structure
struct Config {
  String httpuser;           // username to access web admin
  String httppassword;       // password to access web admin
  int webserverporthttp;     // http port number for web admin
};
Config config;                        // configuration
bool shouldReboot = false;            // schedule a reboot
AsyncWebServer *server;               // initialise webserver

// Colors and palettes
DEFINE_GRADIENT_PALETTE( purple_gp ) {
  0,   0, 212, 255,   //blue
255, 179,   0, 255 }; //purple
DEFINE_GRADIENT_PALETTE( outrun_gp ) {
  0, 141,   0, 100,   //purple
127, 255, 192,   0,   //yellow
255,   0,   5, 255 };  //blue
DEFINE_GRADIENT_PALETTE( greenblue_gp ) {
  0,   0, 255,  60,   //green
 64,   0, 236, 255,   //cyan
128,   0,   5, 255,   //blue
192,   0, 236, 255,   //cyan
255,   0, 255,  60 }; //green
DEFINE_GRADIENT_PALETTE( redyellow_gp ) {
  0,   200, 200,  200,   //white
 64,   255, 218,    0,   //yellow
128,   231,   0,    0,   //red
192,   255, 218,    0,   //yellow
255,   200, 200,  200 }; //white
CRGBPalette16 purplePal = purple_gp;
CRGBPalette16 outrunPal = outrun_gp;
CRGBPalette16 greenbluePal = greenblue_gp;
CRGBPalette16 heatPal = redyellow_gp;
uint8_t colorTimer = 0;

uint8_t peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t prevFFTValue[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t barHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


// using EEPROM for storing values
#define EEPROM_SIZE 5
#define EEPROM_BRIGHTNESS   0
#define EEPROM_GAIN         1
#define EEPROM_SQUELCH      2
#define EEPROM_PATTERN      3
#define EEPROM_DISPLAY_TIME 4
#define EEPROM_MODE 5


// Reboot ESP
void rebootESP(String message) {
  Serial.print("Rebooting ESP32: "); Serial.println(message);
  ESP.restart();
}

// Serial debug routine
void serialDebug() {
  Serial.println("---");
  Serial.print("Mode         : "); Serial.println(mode);
  Serial.print("Pattern      : "); Serial.println(pattern);
  Serial.print("Brightness   : "); Serial.println(brightness);
  Serial.print("Mic Gain     : "); Serial.println(gain);
  Serial.print("HIGH         : ");Serial.println(HIGH);
  Serial.print("LOW          : ");Serial.println(LOW);
  Serial.print("ROTENC_0_SW  : ");Serial.println(digitalRead(ROTENC_0_SW));
  Serial.print("ROTENC_1_SW  : ");Serial.println(digitalRead(ROTENC_1_SW));
  Serial.print("Wifi disabled: ");Serial.println(nowifi);  

  if (digitalRead(ROTENC_0_SW) == LOW && digitalRead(ROTENC_1_SW) == LOW) {
    rebootESP("INFO: Reboot requested by User");
  }
}
