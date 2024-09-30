#include "wifi_handler.h"
#include "global_config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

void initWifi() {
    WiFi.setOutputPower(20.5);
    //if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secundaryDNS)) Serial.println("La configuracion de IP fija fallo");
    WiFi.begin(ssid, password);
    Serial.print("Conectando...");
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500);
        Serial.print(".");
    }
    Serial.print("Conectado con éxito, mi IP es: ");
    Serial.println(WiFi.localIP());
    /*
    WiFi.mode( WIFI_AP );//for AP mode
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Punto de acceso iniciado. IP: ");
    Serial.println(IP);
    */
}

void wifiKeepAlive(){
    if ( WiFi.status() != WL_CONNECTED ){
    int tries = 0;
    WiFi.begin( ssid, password );
    while( WiFi.status() != WL_CONNECTED ) {
        tries++;
        Serial.println( "|" );
        if ( tries == 500 ) ESP.reset();
        delay( 50 );
    }
    WiFi.setOutputPower(20.5);
    Serial.print( "Connected " );
    Serial.println( WiFi.localIP() );
    }
}