#ifndef FS_MANAGER_H
#define FS_MANAGER_H

#include <Arduino.h>

void initFS(void);
bool doFileExists(const char* dir);
bool updateFile(const char* dir, const String* data);
bool updateFile(const char* dir, const char* data);

#endif