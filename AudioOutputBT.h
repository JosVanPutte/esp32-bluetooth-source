
#pragma once
#include "AudioOutput.h"
#include "a2dp_source.h"

static File inputfile;
static uint32_t inputpos;

#define BUFFERSIZE 4096
static uint8_t buffer[BUFFERSIZE];

static int buffered = 0;

int32_t bt_data(uint8_t *data, int32_t len, uint32_t* sr) {
    *sr = 44100;
    if (len < 0 || data == NULL) {
        bt_info("ERROR");
        return 0;
    }
    int cnt = 0;
    if (inputfile) {
        cnt = inputfile.read(data, len);
        if (cnt < len) {
           bt_info("FINISH");
           inputfile.close();
        }
    }
    if (cnt < len) {
        memset(data+cnt, 0, len - cnt);
    }
    return cnt;
}


class AudioOutputBT : public AudioOutput {
    bool started;
public:
    AudioOutputBT(const char * device) {
        a2dp_source_init(device, "");
    }
    void ConsumeFile(File wavfile, uint32_t pos) {
        inputfile = wavfile;
        inputpos = pos;
    }
    uint16_t ConsumeSamples(int16_t* samples, uint16_t count) override {
        Serial.println(count);
        return count;
    }

    bool ConsumeSample(int16_t samples[2]) {
        if (!started) {
            started = true;
            Serial.println("Buffering start");
        }
        if (buffered >= BUFFERSIZE) {
            return false;
        }
        buffer[buffered++] = (samples[0] + 32768) / 257;
        buffer[buffered++] = (samples[1] + 32768) / 257;
        return true;
    }
    bool begin() override {
        Serial.println("Playing start");
        hertz = 44100;
        return true;
    }

    bool isIdle() {
        return buffered == 0;
    }
    void flush() override {
        buffered = 0;
    }

    bool stop() override {
        Serial.println("Stop");
        return true;
    }

    bool SetRate(int hz) override {
        if (hertz != hz) {
            Serial.println("samplerate mismatch");
        }
        return true;
    }

    bool SetChannels(int ch) override {
        if (channels != ch) {
            Serial.print("channels set to ");
            Serial.print(ch);
            channels = ch;
        }
        return true;
    }

};

