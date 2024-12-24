/**
 * MIT License
 * 
 * Copyright (c) 2022 Adrien Grellard
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _DEF_H
  #define _DEF_H

  #include <Arduino.h>
  #if defined(ESP8266)
    extern "C" {
      #include "user_interface.h"
    }
    #include <ESP8266WiFi.h>
    #include <ESP8266mDNS.h>
    #include <ESPAsyncTCP.h>
  #elif defined(ESP32)
    #include <ESPmDNS.h>
    #include <AsyncTCP.h>
    #include <WiFi.h>
    #include <esp_wifi.h>
    #include <Update.h>
  #endif

  // #ifdef FSOK
    #ifdef FILESYSTEM 
      #if defined USE_LITTLEFS
        #if defined(ESP8266)
          #include <LittleFS.h> 
        #elif defined(ESP32)
          #include <FS.h>
          #include <LITTLEFS.h>
        #endif
      #elif defined USE_SPIFFS
        #include <FS.h>
      #endif
    #endif  
  // #endif

  #include <DNSServer.h>
  #include <ArduinoOTA.h>
  #include <ESPAsyncWebServer.h>
  // #include <ESPAsyncUDP.h>  
  #include <altoolslib.h>
  #include <LinkedList.h>

  #ifdef ESP8266
      #define WIFI_getChipId()  ESP.getChipId() 
      #define WM_WIFIOPEN       ENC_TYPE_NONE
  #elif defined(ESP32)
      #define WIFI_getChipId()  (uint32_t)ESP.getEfuseMac()
      #define WM_WIFIOPEN       WIFI_AUTH_OPEN
  #endif

  #ifndef WCEVO_CONNECTED
    #define WCEVO_CONNECTED (WiFi.localIP()[0] != 0 && WiFi.status() == WL_CONNECTED)
  #endif
  // #ifdef ESP8266
  //   #define JSON_BUFFER_SIZE 12000
  // #else
  //   #define JSON_BUFFER_SIZE 20480
  // #endif

  #ifndef WCEVO_DEFINE_DEFAULT
    #ifndef WCEVO_APPSK
      #define WCEVO_APPSK "adsap1234"
    #endif
    #ifndef WCEVO_OTAPSK
      #define WCEVO_OTAPSK "adsota1234"
    #endif
    #ifndef WCEVO_SSID
      #define WCEVO_SSID "routeur-ssid"
    #endif
    #ifndef WCEVO_SSIDPSK
      #define WCEVO_SSIDPSK "routeur-pswd"
    #endif
    #ifndef WCEVO_HOSTNAME
      #define WCEVO_HOSTNAME "esp_wificonnect"
    #endif
  #endif

  #define WCEVO_PORTAL_DEFAULT 0
  #define WCEVO_PORTAL_UI 1
  #ifndef WCEVO_PORTAL
    #define WCEVO_PORTAL WCEVO_PORTAL_UI
  #endif
    
  // #include <esp_idf_version.h>

/*
  // #include "esp_arduino_version.h"
  #ifdef ESP_ARDUINO_VERSION
      // #pragma message "ESP_ARDUINO_VERSION_MAJOR = " STRING(ESP_ARDUINO_VERSION_MAJOR)
      // #pragma message "ESP_ARDUINO_VERSION_MINOR = " STRING(ESP_ARDUINO_VERSION_MINOR)
      // #pragma message "ESP_ARDUINO_VERSION_PATCH = " STRING(ESP_ARDUINO_VERSION_PATCH)
      #define VER_ARDUINO_STR STRING(ESP_ARDUINO_VERSION_MAJOR)  "."  STRING(ESP_ARDUINO_VERSION_MINOR)  "."  STRING(ESP_ARDUINO_VERSION_PATCH)
  #else
      // #include <core_version.h>
      // #pragma message "ESP_ARDUINO_VERSION_GIT  = " STRING(ARDUINO_ESP32_GIT_VER)//  0x46d5afb1
      // #pragma message "ESP_ARDUINO_VERSION_DESC = " STRING(ARDUINO_ESP32_GIT_DESC) //  1.0.6
      #define VER_ARDUINO_STR "Unknown"
      // #pragma message "ESP_ARDUINO_VERSION_REL  = " STRING(ARDUINO_ESP32_RELEASE) //"1_0_6"
  #endif
*/
  #define F_EX(string_literal)  (String(FPSTR(string_literal)).c_str())
  
#endif // DEF_H