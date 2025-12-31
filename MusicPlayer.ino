#include <WiFiManager.h>
#include "microSD.h"
#include "storage.h"
#include "AudioOutputBT.h"
#include "a2dp_source.h"

// #define CYD -- when usign a CYD
// #define RESET

String playLists[20];
String currentSong = "";
int currentLine = 1;
int maxLine = -1;

#ifdef CYD
/*-------- CYD (Cheap Yellow Display) ----------*/

#define TOUCH_CS // This sketch does not use touch, but this is defined to quiet the warning about not defining touch_cs.

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();              // Invoke custom library

#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH 240
#define SPACER 20

int screenCenterX = SCREEN_WIDTH / 2;
uint8_t myFont = 1;
int mySize = 1;
int myDatum = TL_DATUM;
uint16_t myBackgroundColor = TFT_BLACK;
uint16_t myFontColor = TFT_WHITE;

void setupDisplay()
{
  tft.init();
  tft.fillScreen(myBackgroundColor);
  tft.setTextColor(myFontColor, myBackgroundColor);

  tft.setTextFont(myFont);
  tft.setTextSize(mySize);
  tft.setTextDatum(myDatum);
  tft.setTextPadding(SCREEN_WIDTH);
}
void coloredText(String msg, uint16_t color, int y, int font) {
  tft.setTextColor(color, myBackgroundColor);
  tft.drawCentreString(msg, screenCenterX, y, font);
  tft.setTextColor(myFontColor, myBackgroundColor);
}
void showPlayLists() {
  tft.fillScreen(myBackgroundColor);
  coloredText("PlayLists", TFT_BLUE, 5, 2);

  for (int line = 1; line <= maxLine; line++) {
    Serial.println(playLists[line]);
    if (line == currentLine) {
      coloredText(playLists[line], TFT_YELLOW, 30 + SPACER * line, myFont * 2);
    } else {
      tft.drawCentreString(playLists[line], screenCenterX, 30 + SPACER * line, myFont);
    }
  }
   coloredText(currentSong, TFT_YELLOW, SCREEN_HEIGHT - SPACER, myFont);
}
void displaySetup() {
    tft.drawString("wifi: 'MusicPlayer'", 10, tft.height() - 100);
    tft.drawString(WiFi.localIP().toString().c_str(), 10, tft.height() - 100 + SPACER);
}
#else
// -- 0.96 inch I2S oled dislay
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_DEV_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
uint16_t myBackgroundColor = SSD1306_BLACK;
uint16_t myFontColor = SSD1306_WHITE;

void setupDisplay()
{
  display.begin(SSD1306_SWITCHCAPVCC, I2C_DEV_ADDR);
  display.clearDisplay();
  display.setTextColor(myFontColor);
  display.setTextSize(1);
  display.setCursor(4, 28);
  display.println( "Welcome");
  display.display();
}
void showPlayLists() {
  display.clearDisplay();
  display.setTextColor(myFontColor);
  display.setTextSize(1);
  display.setCursor(4, 28);
  display.println(playLists[currentLine]);
  display.println(currentSong);
  display.display();
}
void displaySetup() {
  display.clearDisplay();
  display.setTextColor(myFontColor);
  display.setTextSize(1);
  display.setCursor(4, 28);
  display.println("wifi: 'MusicPlayer'");
  display.println(WiFi.localIP().toString().c_str());
  display.display();
}
#endif

uint32_t parseWAV(File audiofile);

AudioOutputBT *out;

void bt_info(const char* info){
    Serial.printf("bt_info: %s\n", info);
}
void wav_info(const char* info){
    Serial.printf("wav_info: %s", info);
}
nvs_handle_t nvs_storage;
String bluetoothDevice = "";

void showStates() {
  Serial.print("playList nr ");
  Serial.print(currentLine);
  Serial.print(" currentSong ");
  Serial.print(currentSong);
  Serial.print(" on ");
  Serial.println(bluetoothDevice);
}

void setupStates() {
  nvs_storage = initNvs();
#ifndef RESET
  bluetoothDevice = getNonVolatile(nvs_storage, "device");
  currentSong = getNonVolatile(nvs_storage, "song");
  String numStr = getNonVolatile(nvs_storage, "line");
  if (!numStr.isEmpty()) {
    currentLine = atoi(numStr.c_str());
  }
#endif
  if (bluetoothDevice.isEmpty()) {
    WiFiManager wm;
    WiFi.mode(WIFI_STA);
    wm.resetSettings();
    WiFiManagerParameter btDev = WiFiManagerParameter("device", "Bluetooth speaker", bluetoothDevice.c_str(), 50);
    wm.addParameter(&btDev);
    displaySetup();
    if (wm.autoConnect("MusicPlayer") && strlen(btDev.getValue()) && strcmp(bluetoothDevice.c_str(), btDev.getValue())) {
      bluetoothDevice = String(btDev.getValue());
      setNonVolatile(nvs_storage, "device", btDev.getValue());
    }
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
  showStates();
  out = new AudioOutputBT(bluetoothDevice.c_str());
}

void split(String dirs) {
  int idx = dirs.indexOf(";");
  int line = 0;
  Serial.println(dirs);
  while (idx > 0) {
    playLists[line++] = dirs.substring(0, idx);
    if (idx < dirs.length()) {
      dirs = dirs.substring(idx+1, dirs.length());
      Serial.println(dirs);
      idx = dirs.indexOf(";");
    } else {
      Serial.println(dirs);
      Serial.println("at end");
      break;
    }
    Serial.printf("Maxline set to %d\n", line);
    maxLine = line;
  }
}

String getFirstSong() {
  Serial.println(playLists[currentLine]);
  String songsForFirst = listDir(playLists[currentLine]);
  Serial.println(songsForFirst);
  int idx = songsForFirst.indexOf(";");
  if (idx > 0) {
    String retVal = songsForFirst.substring(0, idx);
    Serial.println(retVal);
    return retVal;
  }
  Serial.println("NOT FOUND");
  return "";
}

String getNextSong(const char *preveousSong) {
   Serial.println(playLists[currentLine]);
   String songsForNext = listDir(playLists[currentLine]);
   Serial.println(songsForNext);
   int idx = songsForNext.indexOf(preveousSong);
   if (idx >= 0) {
     String nextSong = songsForNext.substring(idx + strlen(preveousSong) + 1, songsForNext.length());
     idx = nextSong.indexOf(";");
     if (idx > 0) {
      nextSong = nextSong.substring(0, idx);
      Serial.println(nextSong);
      return nextSong;
     }
   }
   Serial.printf("%s no  next song\n", preveousSong);
   return "";
}
/*-------- SETUP & LOOP ----------*/
void setup()
{
  Serial.begin(115200);
  setupDisplay();
  initMicroSD();
  setupStates();
  String dirs = listDir("");
  split(dirs);
  if (maxLine < currentLine) {
    Serial.println("reset player");
    currentLine = 1;
    currentSong = "";
  }
  Serial.println(dirs);
  while (currentSong.isEmpty()) {
    currentSong = getFirstSong();
    if (currentSong.isEmpty()) {
      Serial.print("Cannot find firstSong in this playlist ");
      Serial.println(playLists[currentLine]);
      currentLine += 1;
      if (currentLine > maxLine) {
        currentLine = 1;
      }
    }
  }
  showPlayLists();
}
File playingWav;
bool started = false;

void loop()
{
  bt_loop();
  String playingSong = "/" + playLists[currentLine] + "/" + currentSong ;
  if (!playingWav) {
    if (!started) {
      started = true;
      Serial.printf("Starting %s\n", playingSong.c_str());
      playingWav = SD.open(playingSong.c_str());
      out->ConsumeFile(playingWav, parseWAV(playingWav));
    } else {
      Serial.printf("Finished %s\n", playingSong.c_str());
      currentSong = getNextSong(currentSong.c_str());
      if (currentSong.isEmpty()) {
          currentLine += 1;
          if (currentLine > maxLine) {
            currentLine = 1;
          }
          currentSong = getFirstSong();
      }
      if (!currentSong.isEmpty()) {
        setNonVolatile(nvs_storage, "song", currentSong.c_str());
        started = false;
      }
      showPlayLists();
    }
  }
}
