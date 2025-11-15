
#include "SD.h"
#include "SPI.h"
#include "microSD.h"

#define VSPI_SCLK 18
#define VSPI_MISO 19
#define VSPI_MOSI 23
#define VSPI_SS 5

String listDir(fs::FS &microSD, const char *dirname, bool files) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = microSD.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return "";
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return "";
  }
  String dirList = "";
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (!files) {
        String dir = String(file.name());
        dirList += dir;
        dirList += ";" ;
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
      if (files) {
        String fileName = String(file.name());
        dirList += fileName;
        dirList += ";" ;
      }
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();
  return dirList;
}
File getFile(fs::FS &microSD, const char *fileName) {
  Serial.print("Getting file from directory:");
  Serial.println(fileName);
  File root = microSD.open(fileName);
  if (!root) {
    Serial.print("Failed to open ");
    Serial.println(fileName);
    return root;
  }
  if (!root.isDirectory()) {
    return root;
  }
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
      break;
    }
    file.close();
    file = root.openNextFile();
  }
  if (!file) {
   Serial.println("Something went wrong");
  }
  root.close();
  return file;
}

String listDir(String dir) {
  String dirname = "/" + dir;
  return listDir(SD, dirname.c_str(), !dir.isEmpty());
}
fs::File getFile(String fname) {
  String filename = "/" + fname;
  return getFile(SD, filename.c_str());
}
void initMicroSD() {
  SPI.begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);
  Serial.println("Attempt Card Mount");
  if (!SD.begin(VSPI_SS)) {
    Serial.println("Card Mount Failed");
  } else {
    Serial.println("Card Mount OK");
  }
}

