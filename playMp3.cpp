#include "playMp3.h"

#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputInternalDAC.h>
#include "AudioOutputBT.h"

AudioFileSourceSD *file;
AudioGeneratorMP3 *mp3;
AudioOutput *out;

void setOutput(const char *device) {
  out = new AudioOutputBT(device);
}

MP3STATE playMp3(const char *filename) {
   if (mp3) {
    if (mp3->isRunning()) {
      if (mp3->loop()) {
        // playing
        return MP3_BUSY;
      } else {
        Serial.println("Done");
        mp3->stop();
        return MP3_DONE;
      }
    } else {
      Serial.println("Restart");
      delete mp3;
      mp3 = NULL;
      delete file; 
      file = nullptr; 
    }
   }
   Serial.print("playing ");
   Serial.println(filename);
   file = new AudioFileSourceSD(filename);
   mp3 = new AudioGeneratorMP3();
   mp3->begin(file, out);
   if (mp3->isRunning()) {
     return MP3_STARTED;
   } else {
     Serial.println("NOT RUNNING");
     return MP3_NOT_RUNNING;
   }
}