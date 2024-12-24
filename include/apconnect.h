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
/**************************************************************
  ORIGINAL SOURCE AND INSPIRATION
    https://github.com/tzapu/WiFiManager
    WiFiManager is a library for the ESP8266/Arduino platform
    (https://github.com/esp8266/Arduino) to enable easy
    configuration and reconfiguration of WiFi credentials using a Captive Portal
    inspired by:
    http://www.esp8266.com/viewtopic.php?f=29&t=2520
    https://github.com/chriscook8/esp-arduino-apboot
    https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
    Built by AlexT https://github.com/tzapu

      Ported to Async Web Server by https://github.com/alanswx
      Licensed under MIT license

        Ported for my personal library https://github.com/AdriLighting/espwificonnect_evo.git
        Licensed under MIT license
 **************************************************************/

#ifndef _APCONNECT_H
  #define _APCONNECT_H

  #include "def.h"
  #include "credential.h"
  
  #define SERVERREQUEST AsyncWebServerRequest

  class WCEVO_APconnect 
  {
    typedef std::function<void()> callback_function_t;
      
    WCEVO_credential  * _credential   = nullptr;
    WCEVO_server      * _server       = nullptr;
    AsyncWebServer    * _webserver    = nullptr;
    DNSServer         * _dnsServer    = nullptr;

    boolean   _active                = false;
    byte      _serverInitialized     = 0;



    void setup_webserver(boolean);
    void shutdown();

    #if (WCEVO_PORTAL == WCEVO_PORTAL_UI)
    // region ################################################ WiFiManager_h
      // https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L3345-L3362
      String htmlEntities(String str, bool whitespace = false);
      
      // https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L1988-L2211
      String getInfoData(String id);  
    // endregion >>>> 

    // region ################################################ ESPAsyncWiFiManager_h


      //https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L884-L918
      template<class X>
      void handleRoot(X *request); 

      // https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L1879-L1987   
      void handleInfo_v2(AsyncWebServerRequest *request);   
      void handleInfo_v1(AsyncWebServerRequest *request);   
      String infoAsString();

      //https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L1202-L1225
      void handleReset(AsyncWebServerRequest *request);   
    // endregion >>>>  

      void handleScanReset(AsyncWebServerRequest *request);   
      void handleWifi(AsyncWebServerRequest *request, boolean scan);    
      void handleWifiMulti(AsyncWebServerRequest *request, boolean scan);    
      void handleWifiMod(AsyncWebServerRequest *request);    
      void handleWifiSave(AsyncWebServerRequest *request);   
      void handleWifiSaveMulti(AsyncWebServerRequest *request);   
      void handleWifiSaveMod(AsyncWebServerRequest *request);   
      void handleWifiCo(AsyncWebServerRequest *request);   
      void handleExit(AsyncWebServerRequest *request);   
    #endif

      // https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L1256-L1269
      boolean captivePortal(AsyncWebServerRequest *request);      
  public:

    WCEVO_APconnect() {};
    ~WCEVO_APconnect(){};

    void init(WCEVO_server *, DNSServer*, AsyncWebServer*);
    void setup(boolean v1 =  true, boolean v2 = true, boolean v3 = true);
    boolean get_active();
    void set_active(boolean);
  
    void handleNotFound(AsyncWebServerRequest *request);

  };

#endif // _APCONNECT_H