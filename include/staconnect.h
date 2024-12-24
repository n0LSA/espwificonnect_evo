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

#ifndef _STACONNECT_H
  #define _STACONNECT_H
  #include "def.h"
  #include "credential.h"
  /*
  class WCEVO_esp8266STA : public ESP8266WiFiSTAClass
  {
  public:
    WCEVO_esp8266STA() {};
    ~WCEVO_esp8266STA() {};
    void sav_persistent(const char* ssid, const char *passphrase, const uint8_t* bssid){
      if(!WiFi.enableSTA(true)) {
          // enable STA failed
          Serial.println("----> enable STA failed");
          return ;
      }

      if(!ssid || *ssid == 0x00 || strlen(ssid) > 32) {
          // fail SSID too long or missing!
          Serial.println("----> fail SSID too long or missing!");
          return ;
      }

      int passphraseLen = passphrase == nullptr ? 0 : strlen(passphrase);
      if(passphraseLen > 64) {
          // fail passphrase too long!
          Serial.println("----> fail passphrase too long!");
          return ;
      }

      struct station_config conf;
      conf.threshold.authmode = (passphraseLen == 0) ? AUTH_OPEN : (_useInsecureWEP ? AUTH_WEP : AUTH_WPA_PSK);

      if(strlen(ssid) == 32)
          memcpy(reinterpret_cast<char*>(conf.ssid), ssid, 32); //copied in without null term
      else
          strcpy(reinterpret_cast<char*>(conf.ssid), ssid);

      if(passphrase) {
        if (passphraseLen == 64) // it's not a passphrase, is the PSK, which is copied into conf.password without null term
          memcpy(reinterpret_cast<char*>(conf.password), passphrase, 64);
        else
          strcpy(reinterpret_cast<char*>(conf.password), passphrase);
      } else {
        *conf.password = 0;
      }

      conf.threshold.rssi = -127;
      #ifdef NONOSDK3V0
        conf.open_and_wep_mode_disable = !(_useInsecureWEP || *conf.password == 0);
      #endif

      if(bssid) {
        conf.bssid_set = 1;
        memcpy((void *) &conf.bssid[0], (void *) bssid, 6);
      } else {
        conf.bssid_set = 0;
      }

      ETS_UART_INTR_DISABLE();
      wifi_station_set_config(&conf);
      Serial.println("----> wifi_station_set_config");
      ETS_UART_INTR_ENABLE();
    }
    
  };  
  */
  class WCEVO_STAconnect 
  {
      
    WCEVO_credential  * _credential   = nullptr;
    WCEVO_server      * _server       = nullptr;

    uint32_t  _lastReconnectAttempt  = 0;
    uint8_t   _reconnectAttempt      = 0;
    boolean   _active                = false;
    boolean   _serverInitialized     = 0;   
    boolean   _wasConnected           = false;


    void loop();
    void get_reconnectAttempt(uint8_t &);
    void set_active(boolean);
  public:
    // WCEVO_esp8266STA _esp8266STA;

    WCEVO_STAconnect() {};
    ~WCEVO_STAconnect(){};


    void init(WCEVO_server * server);
    boolean setup();
    void reset();
    void get_lastReconnectAttempt(uint32_t &);
    uint8_t get_reconnectAttempt();
    boolean get_active();
    boolean get_serverInitialized();
    boolean get_wasConnected();
    void set_wasConnected(boolean);
    void set_serverInitialized(boolean);
    uint8_t set_reconnectAttempt(boolean);
    void set_lastReconnectAttempt();

  };

#endif // STACONNECT_H