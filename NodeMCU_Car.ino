#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include "RTClib.h"
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

RTC_DS3231 rtc;

int hora;
int minutos;

int bomba = 16;   //UNO: 6  Wemos: 12    NodeMCU: 15
int circuito1 = 0;
int circuito2 = 14;
int circuito3 = 12;  //UNO: 9  Wemos: 2  NodeMCU: 12
int goteo = 13;   //UNO: 10  Wemos: 15  NodeMCU: 13

void HTTP_handleRoot();              
void HTTP_handleRoot();

//Tiempos de referencia para utilizar con la funcion millis() y poder contar tiempo de forma de no detener el procesador
//son los tiempos de duracion de riego para cada circuito
unsigned long ActualTime;
unsigned long c1Time=240000;
unsigned long c2Time=480000;
unsigned long c3Time=720000;
unsigned long gTime=1620000;
unsigned long endTime=1621000;

//variables que tendran el resultado de la suma de ActualTime + X circuito
unsigned long c1TimeA;
unsigned long c2TimeA;
unsigned long c3TimeA;
unsigned long gTimeA;
unsigned long endTimeA;

int contador = 1; //XD

//string de llaves http
String HT;
String HT2;
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
    file.close();
    }
}

void setup() {
  Serial.begin(115200);
  rtc.begin();
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
     //si no funciona el server
     server.onNotFound ( HTTP_handleRoot );
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
      Serial.print("c1TimeA:");
      Serial.println(c1TimeA);
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
    else if (ActualTime>endTimeA){
      digitalWrite(bomba, HIGH);
      digitalWrite(goteo, HIGH);
      Serial.println("SE TERMINO LA RUTINA");
    }
}

  
void loop() {
    //inicia el server 
    server.handleClient();
    DateTime now = rtc.now();
    //recoje la informacion enviada por la app
    HT = server.arg("HT");
    c1 = server.arg("c1");
    c2 = server.arg("c2");
    c3 = server.arg("c3");
    g = server.arg("g");
    rutine = server.arg("rutine");
    rt=server.arg("r");
    //Llamado de las funciones de control del auto
    
    if (c1 == "ON")circuito_1();
    else if (c2 == "ON") circuito_2();
    else if (c3 == "ON") circuito_3();
    else if (g == "ON") goteo_();
    else if (rutine == "ON")rutina();

  
    //OFF
    else if (c1 == "OFF" && c1s==0){ 
      circuito_1OFF();
      Serial.println(c1);
      
      }
    else if (c2 == "OFF" && c2s==0) circuito_2OFF();
    else if (c3 == "OFF" && c3s==0) circuito_3OFF();
    else if (g == "OFF" && gs==0) goteo_OFF();
    else if (rutine == "OFF" && rs==0) stop_all();
    else if (rt == "ON"){Serial.println("AAA");}
    
    else if (HT != ""){
      if (HT != HT2){
        HT2 = HT; 
        Serial.println(HT);
        LittleFS.remove("/config.json");
        File file = LittleFS.open("/config.json", "w");
        if (!file){
          Serial.println("No funco");
          return;
        }
        file.print(HT);
        delay(1);
        file.close();
        ssd();
      }
    }
    
    //reloj
    //while(hora==7 && minutos==0 || hora==19 && minutos==0){
    while(hora==h1[0] && minutos==h1[1] || hora==h2[0] && minutos==h2[1] || hora==h3[0] && minutos==h3[1] || hora==h4[0] && minutos==h4[1]){
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
      else if (ActualTime>endTimeA){
        digitalWrite(bomba, HIGH);
        digitalWrite(goteo, HIGH);
        Serial.println("SE TERMINO LA RUTINA");
        contador=1;
        break;
      }
  }
}
void HTTP_handleRoot(void) {


  if( server.hasArg("c1") )
    {
       Serial.println(server.arg("c1"));
    }


  if( server.hasArg("c2") )
    {
       Serial.println(server.arg("c2"));
    }


  if( server.hasArg("c3") )
    {
       Serial.println(server.arg("c3"));
    }

  if( server.hasArg("g") )
    {
       Serial.println(server.arg("g"));
    }
  if( server.hasArg("rutine") )
    {
       Serial.println(server.arg("rutine"));
    }
  if( server.hasArg("r") )
    {
       Serial.println(server.arg("r"));
    }
  server.send ( 200, "text/html", "" );

  delay(1);
}