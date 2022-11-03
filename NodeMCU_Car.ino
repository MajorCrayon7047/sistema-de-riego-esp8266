#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
//#include "RTClib.h"
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//RTC_DS3231 rtc;
int hora;
int minutos;
int dia;

const int bomba = 16;   //UNO: 6  Wemos: 12    NodeMCU: 15
const int circuito1 = 0;
const int circuito2 = 14;
const int circuito3 = 12;  //UNO: 9  Wemos: 2  NodeMCU: 12
const int goteo = 13;   //UNO: 10  Wemos: 15  NodeMCU: 13

void HTTP_handleRoot();          
//void HTTP_handleRoot(); MANITO SI NO FUNCA ES CULPA DE ESTO

//Tiempos de referencia para utilizar con la funcion millis() y poder contar tiempo de forma de no detener el procesador
//son los tiempos de duracion de riego para cada circuito
unsigned long ActualTime;
unsigned long c1Time=240000; 
unsigned long c2Time=480000;
unsigned long c3Time=720000;
unsigned long gTime=1620000;
unsigned long endTime=1;

unsigned long ultima=0;

//variables que tendran el resultado de la suma de ActualTime + duracion X circuito
unsigned long c1TimeA;
unsigned long c2TimeA;
unsigned long c3TimeA;
unsigned long gTimeA;
unsigned long endTimeA;

bool contador = 1; 
bool contador2 = false;
//string de llaves http
String JsonConfig;
String JsonConfig2;
String c1;
String c2;
String c3;
String g;
String rutine;
String rt;
ESP8266WebServer server(80); //es la quinta vez

int c1s;
int c2s;
int c3s;
int gs;
int rs;

//UTC*60*60   -3*60*60  
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "south-america.pool.ntp.org");

const char* ssid = "Flia Perez";
const char* password = "ponecualquiera";
//const char* ssid = "Notebooks ETEC";
//const char* password = "373k123*";

//ip fija para no tener que ir adivinando
IPAddress local_IP(192,168,54,200);
IPAddress gateway(192,168,54,1);
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(8,8,8,8);
IPAddress secundaryDNS(8,8,4,4);

//IPAddress local_IP(10,54,3,200);
//IPAddress gateway(192,168,40,8);
//IPAddress subnet(255,255,255,0);
//IPAddress primaryDNS(8,8,8,8);
//IPAddress secundaryDNS(8,8,4,4);

//horarios max 4
int h1[2] = {06, 00};
int h2[2] = {18, 00};
int h3[2] = {04, 00};
int h4[2] = {02, 00};

//dias:
bool days[7]= {true, true, true, true, true, true, true};
// if (days[day]==true){XD}

void ssd(){
  if (LittleFS.exists("/config.json") == true){
    File file = LittleFS.open("/config.json", "r");
    DynamicJsonBuffer  jsonBuffer(512);
    JsonObject& json = jsonBuffer.parseObject(file);
    if (!json.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    Serial.print("la wea funciono:");
    Serial.println(json["h1"][0].as<int>());
    h1[0] = json["h1"][0];
    h1[1] = json["h1"][1];
    h2[0] = json["h2"][0];
    h2[1] = json["h2"][1];
    h3[0] = json["h3"][0];
    h3[1] = json["h3"][1];
    h4[0] = json["h4"][0];
    h4[1] = json["h4"][1];

    c1Time = json["duration"][0];
    c2Time = json["duration"][1];
    c3Time = json["duration"][2];
    gTime = json["duration"][3];

    for (int i = 0; i <= 6; i++){
      days[i] = json["days"][i];
    }
    file.close();
    }
}


void setup() {
  Serial.begin(115200);
  //rtc.begin();
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
// Conectar a wifi 
  if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secundaryDNS)){
    Serial.println("La configuracion de IP fija fallo, ptm");
  }
  WiFi.begin(ssid, password);

  Serial.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  Serial.print("Conectado con éxito, mi IP es: ");
  Serial.println(WiFi.localIP());
 
 // Starting WEB-server 
     //prende el server
     server.on ( "/", HTTP_handleRoot );
     server.on("/json", []() {
        if (LittleFS.exists("/config.json") == true){
          File file = LittleFS.open("/config.json", "r");
          server.send(200, "text/plain", file.readString() );
          Serial.println("YES");
      }});
     server.on("/live", [](){
        server.send(200, "text/plain", "ALIVE");
     });
     //si no funciona el server
     server.onNotFound( HTTP_handleRoot );
     server.begin();
     pinMode(circuito1, OUTPUT);
     pinMode(circuito2, OUTPUT);
     pinMode(circuito3, OUTPUT);
     pinMode(goteo, OUTPUT);
     pinMode(bomba, OUTPUT);
    
     //arrancar todo apagado:
     digitalWrite(bomba,HIGH);
     digitalWrite(circuito1,HIGH);
     digitalWrite(circuito2,HIGH);
     digitalWrite(circuito3,HIGH);
     digitalWrite(goteo,HIGH);
    
     timeClient.begin();
     timeClient.setTimeOffset(-10800);
     ssd();
}

// Circuitos
void circuito_1(){ 
  digitalWrite(bomba, LOW);
  digitalWrite(circuito1,LOW);
  Serial.println("CIRCUITO 1 ENCENDIDO");
  c1s=0;
  }

void circuito_2(){ 
  digitalWrite(bomba, LOW);
  digitalWrite(circuito2,LOW);
  Serial.println("CIRCUITO 2 ENCENDIDO");
  c2s=0;
  }

void circuito_3(){ 
  digitalWrite(bomba, LOW);
  digitalWrite(circuito3,LOW);
  Serial.println("CIRCUITO 3 ENCENDIDO");
  c3s=0;
  }
void goteo_(){ 
  digitalWrite(bomba, LOW);
  digitalWrite(goteo,LOW);
  Serial.println("GOTEO ENCENDIDO");
  gs=0;
  }

//OFF
void circuito_1OFF(){ 
  digitalWrite(bomba, HIGH);
  digitalWrite(circuito1,HIGH);
  Serial.println("CIRCUITO 1 APAGADO");
  c1="NADA";
  Serial.println(c1);
  c1s=1;
  }

void circuito_2OFF(){ 
  digitalWrite(bomba, HIGH);
  digitalWrite(circuito2,HIGH);
  Serial.println("CIRCUITO 2 APAGADO");
  c2="NADA";
  c2s=1;
  }

void circuito_3OFF(){ 
  digitalWrite(bomba, HIGH);
  digitalWrite(circuito3,HIGH);
  Serial.println("CIRCUITO 3 APAGADO");
  c3="NADA";
  c3s=1;
  }

void goteo_OFF(){ 
  digitalWrite(bomba, HIGH);
  digitalWrite(goteo,HIGH);
  Serial.println("GOTEO APAGADO");
  g="NADA";
  gs=1;
  }

void stop_all(){  
  digitalWrite(bomba, HIGH);
  digitalWrite(circuito1, HIGH);
  digitalWrite(circuito2, HIGH);
  digitalWrite(circuito3, HIGH);
  digitalWrite(goteo, HIGH);
  rutine="NADA";
  contador=1;
  Serial.println("pues si llego aca de alguna forma");
  rs=1;
 }

void rutina(){
  rs=0;
  ActualTime = millis();
    if (contador==1){
      c1TimeA = c1Time+ActualTime;
      c2TimeA = c2Time+ActualTime;
      c3TimeA = c3Time+ActualTime;
      gTimeA = gTime+ActualTime;
      contador=0;
      }
    
    if(ActualTime<=c1TimeA){
      Serial.println("Circuito 1 encendido");
      digitalWrite(bomba, LOW);
      digitalWrite(circuito1,LOW);
    }
    else if(ActualTime<=c2TimeA){
      Serial.println("Circuito 1 apagado");
      Serial.println("Circuito 2 encendido");
      digitalWrite(circuito1, HIGH);
      digitalWrite(circuito2, LOW);
    }
    else if(ActualTime<=c3TimeA){
      Serial.println("Circuito 1 apagado");
      Serial.println("Circuito 2 apagado");
      Serial.println("Circuito 3 encendido");
      digitalWrite(circuito2, HIGH);
      digitalWrite(circuito3, LOW);
    }
    else if(ActualTime<=gTimeA){
      Serial.println("Circuito 1 apagado");
      Serial.println("Circuito 2 apagado");
      Serial.println("Circuito 3 apagado");
      Serial.println("GOTEO encendido");
      digitalWrite(circuito3, HIGH);
      digitalWrite(goteo, LOW); 
    }
    else if ( ActualTime>endTimeA){
      digitalWrite(bomba, HIGH);
      digitalWrite(goteo, HIGH);
      Serial.println("SE TERMINO LA RUTINA");
    }
}

  
void loop() {
    //inicia el server 
    server.handleClient();
    //DateTime now = rtc.now();
    //recoje la informacion enviada por la app
    JsonConfig = server.arg("HT");
    c1 = server.arg("c1");
    c2 = server.arg("c2");
    c3 = server.arg("c3");
    g = server.arg("g");
    rutine = server.arg("rutine");
    rt = server.arg("r");

    if (c1 == "ON")circuito_1();
    else if (c1 == "OFF" && c1s==0)circuito_1OFF();

    if (c2 == "ON") circuito_2();
    else if (c2 == "OFF" && c2s==0) circuito_2OFF();
    
    if (c3 == "ON") circuito_3();
    else if (c3 == "OFF" && c3s==0) circuito_3OFF();

    if (g == "ON") goteo_();
    else if (g == "OFF" && gs==0) goteo_OFF();

    if (rutine == "ON")rutina();
    else if (rutine == "OFF" && rs==0) stop_all();
  

    else if (JsonConfig != ""){
      if (JsonConfig != JsonConfig2){
        JsonConfig2 = JsonConfig; 
        Serial.println(JsonConfig);
        LittleFS.remove("/config.json");
        File file = LittleFS.open("/config.json", "w");
        if (!file){
          Serial.println("No funco");
          return;
        }
        file.print(JsonConfig);
        delay(1);
        file.close();
        ESP.reset();
      }
    }
    //reloj
    if (millis()>=ultima){
    ultima = millis() + 500;
    timeClient.update();
    dia = timeClient.getDay();
    hora = timeClient.getHours();
    minutos = timeClient.getMinutes();
    //Serial.print(dia);
    //Serial.print("\t");
    //Serial.print(hora);
    //Serial.print(":");
    //Serial.println(minutos);
    }
    //while(hora==7 && minutos==0 || hora==19 && minutos==0){
    if (days[dia] == true){
    if (hora==h1[0] && minutos==h1[1] || hora==h2[0] && minutos==h2[1] || hora==h3[0] && minutos==h3[1] || hora==h4[0] && minutos==h4[1]){
      contador2 = true;
      }
    }
    if (contador2 == true){
        ActualTime = millis();
        
        if (contador==1){
          c1TimeA = c1Time+ActualTime;
          Serial.print("c1TimeA:");
          Serial.println(c1TimeA);
          c2TimeA = c2Time+ActualTime;
          c3TimeA = c3Time+ActualTime;
          gTimeA = gTime+ActualTime;
          contador=0;
          }
        digitalWrite(bomba, LOW);
        if(ActualTime<=c1TimeA){
          Serial.println("Circuito 1 encendido");
          
          digitalWrite(circuito1,LOW);
        }
        else if(ActualTime<=c2TimeA){
          Serial.println("Circuito 1 apagado");
          Serial.println("Circuito 2 encendido");
          digitalWrite(circuito1, HIGH);
          digitalWrite(circuito2, LOW);
        }
        else if(ActualTime<=c3TimeA){
          Serial.println("Circuito 1 apagado");
          Serial.println("Circuito 2 apagado");
          Serial.println("Circuito 3 encendido");
          digitalWrite(circuito2, HIGH);
          digitalWrite(circuito3, LOW);
        }
        else if(ActualTime<=gTimeA){
          Serial.println("Circuito 1 apagado");
          Serial.println("Circuito 2 apagado");
          Serial.println("Circuito 3 apagado");
          Serial.println("GOTEO encendido");
          digitalWrite(circuito3, HIGH);
          digitalWrite(goteo, LOW); 
        }
        else if (ActualTime>endTimeA){
          digitalWrite(bomba, HIGH);
          digitalWrite(goteo, HIGH);
          Serial.println("SE TERMINO LA RUTINA");
          contador=1;
          contador2=false;
        }
    }
}
void HTTP_handleRoot(void) {
  server.send ( 200, "text/html", "" );
  delay(1);
}