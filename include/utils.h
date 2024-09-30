#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

//FS utils
void initFS(void);
bool doFileExists(const char* dir);
bool updateFile(const char* dir, const String* data);
bool updateFile(const char* dir, const char* data);
String readFile(const char* dir);

//other utils
void updatePins(bool onlyCheck = false);

#endif // UTILS_H