#include "persistence.h"
#include <LittleFS.h>

boolean isOk = false;

void initFileSystem() {
  if (LittleFS.begin()) {
    Serial.println("Flash FS available!");
    isOk = true;
  } else {
    Serial.println("Flash FS initialisation failed!");
  }

  listFiles();
}
void listFiles() {
  if (!isOk) return;
  Serial.println("Flash FS files found:");

  File root = LittleFS.open("/", "r");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    Serial.printf("- %s, %d bytes\n", entry.name(), entry.size());
    entry.close();
  }
}

const char *readData(const char *filename) {
   if (!isOk) return NULL;
   File file = LittleFS.open(filename, "r");
   char *buffer = (char *) malloc(file.size());
   if (file.available()) {
    file.readBytes(buffer, file.size());
   }
   file.close();
   Serial.printf("buffer %d chars\n", strlen(buffer));
   return buffer;
}

bool fileSystemOk() {
  return isOk;
}