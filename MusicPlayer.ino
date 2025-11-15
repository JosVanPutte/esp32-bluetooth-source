#define TOUCH_CS // This sketch does not use touch, but this is defined to quiet the warning about not defining touch_cs.

/*-------- CYD (Cheap Yellow Display) ----------*/
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include "microSD.h"
#include "playMp3.h"
#include "storage.h"
#include "connectBT.h"
#include <WiFiManager.h>

TFT_eSPI tft = TFT_eSPI();              // Invoke custom library

#define CYCLE_MS 1000000
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH 240
#define SPACER 20

int screenCenterX = SCREEN_WIDTH / 2;
uint8_t myFont = 1;
int mySize = 1;
int myDatum = TL_DATUM;
uint16_t myBackgroundColor = TFT_BLACK;
uint16_t myFontColor = TFT_WHITE;
int currentLine = 1;

// for connectBT
void log(const char *msg, float t) {
  Serial.print(msg);
  Serial.print(" at ");
  Serial.println(t);
}

void SetupCYD()
{
  tft.init();
  tft.fillScreen(myBackgroundColor);
  tft.setTextColor(myFontColor, myBackgroundColor);

  tft.setTextFont(myFont);
  tft.setTextSize(mySize);
  tft.setTextDatum(myDatum);
  tft.setTextPadding(SCREEN_WIDTH);
}
String bluetoothDevice = "";

void setupBTSink() {
  WiFiManager wm;
  WiFi.mode(WIFI_STA);
  nvs_handle_t nvs_handle = initNvs();
  bluetoothDevice = getNonVolatile(nvs_handle, "device");
  if (bluetoothDevice.isEmpty()) {
    wm.resetSettings();
  }
  WiFiManagerParameter btDev = WiFiManagerParameter("device", "Bluetooth speaker", bluetoothDevice.c_str(), 50);
  wm.addParameter(&btDev);
  tft.drawString("wifi: 'MusicPlayer'", 10, tft.height() - 100);
  if (wm.autoConnect("MusicPlayer") && strlen(btDev.getValue()) && strcmp(bluetoothDevice.c_str(), btDev.getValue())) {
    bluetoothDevice = String(btDev.getValue());
    setNonVolatile(nvs_handle, "device", btDev.getValue());
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.print("Speaker ");
  Serial.println(bluetoothDevice);
}

int maxLine = -1;
String playLists[20];

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
    Serial.printf("Maxline set to %d", line);
    maxLine = line;
  }
}
String currentSong = "";

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
  Serial.println("FAILED");
  return "NOT FOUND";
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
/*-------- SETUP & LOOP ----------*/
void setup()
{
  Serial.begin(115200);
  SetupCYD();
  setupBTSink();
  initMicroSD();
  String dirs = listDir("");
  split(dirs);
  Serial.println(dirs);
  currentSong = getFirstSong();
  showPlayLists();
}

long reported = 0;
void loop()
{
  String playingSong = "/" + playLists[currentLine] + "/" + currentSong;
  MP3STATE mp3State = playMp3(playingSong.c_str());
  if (mp3State == MP3_STARTED) {
    Serial.print(playingSong);
    Serial.println(" started");
  }
  if (mp3State == MP3_DONE) {
    Serial.print(playingSong);
    Serial.println(" finished");
    String nextSong = getNextSong(currentSong.c_str());
    Serial.print("Next song is ");
    Serial.println(nextSong);
    if (nextSong.isEmpty()) {
      Serial.println("Playlist done");
      currentLine++;
      currentSong = getFirstSong();
      showPlayLists();
    } else {
      currentSong = nextSong;
    }
    coloredText( currentSong, TFT_YELLOW, SCREEN_HEIGHT - SPACER, myFont);
  }
  if (!reported && !bluetoothDevice.isEmpty()) {
    connectBT(bluetoothDevice, "100");
  }
  if (reported++ > CYCLE_MS) {
    reported = 1;
    Serial.print("playing ");
    Serial.println(currentSong);
  }
}
