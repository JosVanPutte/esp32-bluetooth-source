#include "playMp3.h"

#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputInternalDAC.h>

AudioFileSourceSD *file;
AudioGeneratorMP3 *mp3;
AudioOutputInternalDAC * out;

MP3STATE playMp3(const char *filename) {
   if (mp3) {
    if (mp3->isRunning()) {
      if (mp3->loop()) {
        // playing
        return MP3_BUSY;
      } else {
        Serial.println(" Done");
        mp3->stop();
        return MP3_DONE;
      }
    }
    delete mp3;
    mp3 = nullptr;
    if (file) { delete file; file = nullptr; }
    if (out)  { delete out;  out = nullptr; }
   }
   Serial.print("playing ");
   Serial.println(filename);
   file = new AudioFileSourceSD(filename);
   out = new AudioOutputInternalDAC();
   mp3 = new AudioGeneratorMP3();
   mp3->begin(file, out);
   return MP3_STARTED;
}