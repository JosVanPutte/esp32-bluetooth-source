
enum MP3STATE { MP3_BUSY, MP3_DONE, MP3_STARTED, MP3_NOT_RUNNING, MP3_INIT};

void setOutput(const char *device);

MP3STATE playMp3(const char *filename);