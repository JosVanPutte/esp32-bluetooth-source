
#include "connectBT.h"

#include <math.h>
#include <cstdio> 

#define c3_frequency  130.81
#define SAMPLES_PER_SEC = 
BluetoothA2DPSource a2dp_source;

static float trigger_time = 0.0;
static float m_time = 0.0;

int32_t get_data_frames(Frame *frame, int32_t frame_count) {
    float m_amplitude = 10000.0;  // -32,768 to 32,767
    float m_deltaTime = 1.0 / 04410.0;
    float m_phase = 0.0;
    float pi_2 = M_PI * 2.0;
    // fill the channel data
    for (int sample = 0; sample < frame_count; ++sample) {
        float angle = pi_2 * c3_frequency * m_time + m_phase;
        frame[sample].channel1 = m_amplitude * sin(angle);
        frame[sample].channel2 = frame[sample].channel1;
        m_time += m_deltaTime;
    }
    if (m_time > trigger_time + 15.0) {
      log("trigger", m_time);
      trigger_time = m_time;
      delay(1);
    }
    return frame_count;
}

void connectBT(String device, String volume) {
  a2dp_source.set_auto_reconnect(true);
  a2dp_source.set_data_callback_in_frames(get_data_frames);
  a2dp_source.set_volume(atoi(volume.c_str()));
  log("Start output", trigger_time);
  a2dp_source.start(device.c_str());
}