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

/**
 * WiFiManager.cpp
 * 
 * WiFiManager, a library for the ESP8266/Arduino platform
 * for configuration of WiFi credentials using a Captive Portal
 * 
 * @author Creator tzapu
 * @author tablatronix
 * @version 0.0.0
 * @license MIT
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




#include "../include/wcevo_wifimanager.h"
#include "../include/al_wificonnectevo.h"




/*
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L3578-L3607
*/
String WiFiManagerCPY::WiFi_SSID(bool persistent) const{

    #ifdef ESP8266
    struct station_config conf;
    if(persistent) wifi_station_get_config_default(&conf);
    else wifi_station_get_config(&conf);

    char tmp[33]; //ssid can be up to 32chars, => plus null term
    memcpy(tmp, conf.ssid, sizeof(conf.ssid));
    tmp[32] = 0; //nullterm in case of 32 char ssid
    return String(reinterpret_cast<char*>(tmp));
    
    #elif defined(ESP32)
    if(persistent){
      wifi_config_t conf;
      esp_wifi_get_config(WIFI_IF_STA, &conf);
      return String(reinterpret_cast<const char*>(conf.sta.ssid));
    }
    else {
      if(WiFiGenericClass::getMode() == WIFI_MODE_NULL){
          return String();
      }
      wifi_ap_record_t info;
      if(!esp_wifi_sta_get_ap_info(&info)) {
          return String(reinterpret_cast<char*>(info.ssid));
      }
      return String();
    }
    #endif
}

// set mode ignores WiFi.persistent 
bool WiFiManagerCPY::WiFi_Mode(WiFiMode_t m,bool persistent) {
    bool ret;
    #ifdef ESP8266
      if((wifi_get_opmode() == (uint8) m ) && !persistent) {
          return true;
      }
      ETS_UART_INTR_DISABLE();
      if(persistent) ret = wifi_set_opmode(m);
      else ret = wifi_set_opmode_current(m);
      ETS_UART_INTR_ENABLE();
    return ret;
    #elif defined(ESP32)
      if(persistent && esp32persistent) WiFi.persistent(true);
      ret = WiFi.mode(m); // @todo persistent check persistant mode , NI
      if(persistent && esp32persistent) WiFi.persistent(false);
      return ret;
    #endif
}
bool WiFiManagerCPY::WiFi_Mode(WiFiMode_t m) {
  return WiFi_Mode(m,false);
}

// sta disconnect without persistent
bool WiFiManagerCPY::WiFi_Disconnect() {
    #ifdef ESP8266
      if((WiFi.getMode() & WIFI_STA) != 0) {
          bool ret;
          ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "WiFi station disconnect\n"); // DEBUG_DEV
          ETS_UART_INTR_DISABLE(); // @todo probably not needed
          ret = wifi_station_disconnect();
          ETS_UART_INTR_ENABLE();        
          return ret;
      }
    #elif defined(ESP32)
      ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "WiFi station disconnect\n"); // DEBUG_DEV
      return WiFi.disconnect(); // not persistent atm
    #endif
    return false;
}

// toggle STA without persistent
bool WiFiManagerCPY::WiFi_enableSTA(bool enable,bool persistent) {
#ifdef WM_DEBUG_LEVEL
    DEBUG_WM(DEBUG_DEV,F("WiFi_enableSTA"),(String) enable? "enable" : "disable");
    #endif
    #ifdef ESP8266
      WiFiMode_t newMode;
      WiFiMode_t currentMode = WiFi.getMode();
      bool isEnabled         = (currentMode & WIFI_STA) != 0;
      if(enable) newMode     = (WiFiMode_t)(currentMode | WIFI_STA);
      else newMode           = (WiFiMode_t)(currentMode & (~WIFI_STA));

      if((isEnabled != enable) || persistent) {
          if(enable) {
          #ifdef WM_DEBUG_LEVEL
            if(persistent) DEBUG_WM(DEBUG_DEV,F("enableSTA PERSISTENT ON"));
            #endif
              return WiFi_Mode(newMode,persistent);
          }
          else {
              return WiFi_Mode(newMode,persistent);
          }
      } else {
          return true;
      }
    #elif defined(ESP32)
      bool ret;
      if(persistent && esp32persistent) WiFi.persistent(true);
      ret =  WiFi.enableSTA(enable); // @todo handle persistent when it is implemented in platform
      if(persistent && esp32persistent) WiFi.persistent(false);
      return ret;
    #endif
}

bool WiFiManagerCPY::WiFi_enableSTA(bool enable) {
  return WiFi_enableSTA(enable,false);
}

bool WiFiManagerCPY::WiFi_eraseConfig() {
    #ifdef WM_DEBUG_LEVEL
    DEBUG_WM(DEBUG_DEV,F("WiFi_eraseConfig"));
    #endif

    #ifdef ESP8266
      #ifndef WM_FIXERASECONFIG 
        return ESP.eraseConfig();
      #else
        // erase config BUG replacement
        // https://github.com/esp8266/Arduino/pull/3635
        const size_t cfgSize = 0x4000;
        size_t cfgAddr = ESP.getFlashChipSize() - cfgSize;

        for (size_t offset = 0; offset < cfgSize; offset += SPI_FLASH_SEC_SIZE) {
            if (!ESP.flashEraseSector((cfgAddr + offset) / SPI_FLASH_SEC_SIZE)) {
                return false;
            }
        }
        return true;
      #endif
    #elif defined(ESP32)

      bool ret;
      WiFi.mode(WIFI_AP_STA); // cannot erase if not in STA mode !
      WiFi.persistent(true);
      ret = WiFi.disconnect(true,true);
      delay(500);
      WiFi.persistent(false);
      return ret;
    #endif
}

uint8_t WiFiManagerCPY::WiFi_softap_num_stations(){
  #ifdef ESP8266
    return wifi_softap_get_station_num();
  #elif defined(ESP32)
    return WiFi.softAPgetStationNum();
  #endif
}

bool WiFiManagerCPY::WiFi_hasAutoConnect(){
  return WiFi_SSID(true) != "";
}

String WiFiManagerCPY::WiFi_psk(bool persistent) const {
    #ifdef ESP8266
    struct station_config conf;

    if(persistent) wifi_station_get_config_default(&conf);
    else wifi_station_get_config(&conf);

    char tmp[65]; //psk is 64 bytes hex => plus null term
    memcpy(tmp, conf.password, sizeof(conf.password));
    tmp[64] = 0; //null term in case of 64 byte psk
    return String(reinterpret_cast<char*>(tmp));
    
    #elif defined(ESP32)
    // only if wifi is init
    if(WiFiGenericClass::getMode() == WIFI_MODE_NULL){
      return String();
    }
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    return String(reinterpret_cast<char*>(conf.sta.password));
    #endif
}


/*
  boolean WCEVO_APconnect::isIp(String str)
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L3306-L3315
  https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L1330-L1342

  String WCEVO_APconnect::toStringIp(IPAddress ip)
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L3317-L3325
  https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L1344-L1354

*/
// is this an IP?
boolean WiFiManagerCPY::isIp(String str)
{
  for (unsigned int i = 0; i < str.length(); i++)
  {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9'))
    {
      return false;
    }
  }
  return true;
}

// IP to String?
String WiFiManagerCPY::toStringIp(IPAddress ip)
{
  String res = "";
  for (int i = 0; i < 3; i++)
  {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
} 

/**
 * [getWLStatusString description]
 * @access public
 * @param  {[type]} uint8_t status        [description]
 * @return {[type]}         [description]
 */
String WiFiManagerCPY::getWLStatusString(uint8_t status){
  if(status <= 7) return WIFI_STA_STATUS[status];
  return FPSTR(S_NA);
}

String WiFiManagerCPY::getModeString(uint8_t mode){
  if(mode <= 3) return WIFI_MODES[mode];
  return FPSTR(S_NA);
}



// @todo refactor this up into seperate functions
// one for connecting to flash , one for new client
// clean up, flow is convoluted, and causes bugs
void WiFiManagerCPY::connectWifi(String ssid, String pass, bool connect) {
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "Connecting as wifi client...\n");
  if(_cleanConnect) WiFi_Disconnect(); // disconnect before begin, in case anything is hung, this causes a 2 seconds delay for connect
  if (ssid != "") {
    wifiConnectNew(ssid,pass,connect);
  }
  else {
    if (WiFi_hasAutoConnect()) {
      wifiConnectDefault();
    }
    else {
      ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "No wifi saved, skipping\n");
    }
  }
}

/**
 * connect to a new wifi ap
 * @since $dev
 * @param  String ssid 
 * @param  String pass 
 * @return bool success
 * @return connect only save if false
 */
bool WiFiManagerCPY::wifiConnectNew(String ssid, String pass,bool connect){
  bool ret = false;
  // DEBUG_WM(DEBUG_DEV,F("CONNECTED: "),WiFi.status() == WL_CONNECTED ? "Y" : "NO");
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "Connecting to NEW AP: %s\n",ssid.c_str());
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "Using Password: %s\n",pass.c_str()); // DEBUG_DEV
  WiFi_enableSTA(true,storeSTAmode); // storeSTAmode will also toggle STA on in default opmode (persistent) if true (default)
  WiFi.persistent(true);
  ret = WiFi.begin(ssid.c_str(), pass.c_str(), 0, NULL, connect);
  WiFi.persistent(false);
  if(!ret) ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "[ERROR] wifi begin failed\n"); // DEBUG_ERROR
  return ret;
}

/**
 * connect to stored wifi
 * @since dev
 * @return bool success
 */
bool WiFiManagerCPY::wifiConnectDefault(){
  bool ret = false;

  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "Connecting to SAVED AP: %s\n", WiFi_SSID(true).c_str());
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "Using Password: %s\n", WiFi_psk(true).c_str()); // DEBUG_DEV

  ret = WiFi_enableSTA(true,storeSTAmode);
  delay(500); // THIS DELAY ?

  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "Mode after delay: %s\n", getModeString(WiFi.getMode()).c_str()); // DEBUG_DEV
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "[ERROR] wifi enableSta failed\n"); // DEBUG_ERROR

  ret = WiFi.begin();

  if(!ret) ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "[ERROR] wifi begin failed\n"); // DEBUG_ERROR


  return ret;
}

/*
uint8_t WiFiManager::waitForConnectResult(uint32_t timeout) {
  if (timeout == 0){
    #ifdef WM_DEBUG_LEVEL
    DEBUG_WM(F("connectTimeout not set, ESP waitForConnectResult..."));
    #endif
    return WiFi.waitForConnectResult();
  }

  unsigned long timeoutmillis = millis() + timeout;
  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(DEBUG_VERBOSE,timeout,F("ms timeout, waiting for connect..."));
  #endif
  uint8_t status = WiFi.status();
  
  while(millis() < timeoutmillis) {
    status = WiFi.status();
    // @todo detect additional states, connect happens, then dhcp then get ip, there is some delay here, make sure not to timeout if waiting on IP
    if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
      return status;
    }
    #ifdef WM_DEBUG_LEVEL
    DEBUG_WM (DEBUG_VERBOSE,F("."));
    #endif
    delay(100);
  }
  return status;
}



*/