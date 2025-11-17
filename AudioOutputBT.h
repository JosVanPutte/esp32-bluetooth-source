
#pragma once
#include "AudioOutput.h"
#include "a2dp_source.h"

#define BUFFERSIZE 4096
static uint8_t buffer[BUFFERSIZE];

static int buffered = 0;

bool fetching = false;
bool core_fetch = false;
bool core_put = false;

int32_t bt_data(uint8_t *data, int32_t len, uint32_t* sr) {
    if (!core_fetch) {
      core_fetch = true;
      Serial.print("FETCHING CORE ");
      Serial.println(xPortGetCoreID());
    }
    fetching = true;
    int cnt = len;
    *sr = 44100;
    if (cnt > buffered) {
       Serial.println(buffered);
       cnt = buffered;
    }
    memmove(data, buffer, cnt);
    buffered -= cnt;
    memmove(buffer, buffer + cnt, buffered);
    fetching = false;
    return cnt;
}


class AudioOutputBT : public AudioOutput {
    void initDevice(const char *deviceName) {
        a2dp_source_init((const char *)deviceName, "");
        Serial.println("BlueTooth started");
    }
public:
    AudioOutputBT(const char * device) {
        initDevice(device);
    }
    uint16_t ConsumeSamples(int16_t* samples, uint16_t count) override {
        Serial.println(count);
        return count;
    }

    bool ConsumeSample(int16_t samples[2]) {
        
        if (buffered >= BUFFERSIZE - 2 || fetching) {
            if (!core_put) {
              core_put = true;
              Serial.print("PUTTING CORE ");
              Serial.print(xPortGetCoreID());
              Serial.print(" buffered ");
              Serial.println(buffered);
            }
            return false;
        }
        buffer[buffered++] = (samples[0] + 32768) / 257;
        buffer[buffered++] = (samples[1] + 32768) / 257;
        return true;
    }
    bool begin() override {
        hertz = 44100;
        return true;
    }

    void flush() override {
        buffered = 0;
    }

    bool stop() override {
        flush();
        return true;
    }

    bool SetRate(int hz) override {
        if (hertz != hz) {
            hertz = hz;
            flush();
        }
        return true;
    }

    bool SetChannels(int ch) override {
        if (channels != ch) {
            channels = ch;
            flush();
        }
        return true;
    }

};

