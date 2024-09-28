#include <LittleFS.h>

void initFS(void){
    Serial.println("Iniciando LittleFS");
    int8_t tries = 0;
    while(!LittleFS.begin()){
        Serial.printf("An Error has occurred while mounting LittleFS, retrying in 2 seconds, try number %d\n", tries);
        delay(2000);
        tries++;
        if(tries == 10) ESP.restart();
    }
}

bool doFileExists(const char* dir){
    return LittleFS.exists(dir);
}

bool updateFile(const char* dir, const String* data){
    File file = LittleFS.open(dir, "w");
    if(!file) return false;
    file.print(*data);
    Serial.println("Se guardaron los siguientes datos: " + *data);
    return true;
}
bool updateFile(const char* dir, const char* data){
    File file = LittleFS.open(dir, "w");
    if(!file) return false;
    file.print(data);
    Serial.println("Se guardaron los siguientes datos: " + *data);
    return true;
}
