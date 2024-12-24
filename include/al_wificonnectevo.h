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
/*
  espwificonnect_evo is a library for the framework-arduinoespressif8266/32 
    configuration and reconfiguration of WiFi credentials using a Captive Portal


  library used for my personalproject
    fetures requiered 
      - asyncrone mod
      - multiple credential with best ssid rssi
      - STA + AP || STA || AP
      - captive portal 
        - station mod
        - credential || multiple  
        - info
          - ip
          - hostname

  inspired by:
    library:
      https://github.com/Aircoookie/WLED/blob/main/wled00/wled.cpp#L684-L777
      https://github.com/tzapu/WiFiManager
      https://github.com/alanswx/ESPAsyncWiFiManager
      https://gist.github.com/tablatronix/497f3b299e0f212fc171ac5662fa7b42
      framework-arduinoespressif8266 - https://github.com/esp8266/Arduino
        ESP8266WiFi - ESP8266WiFiMulti.h
      https://github.com/disk91/esp8266-sigfox-trackr
      https://github.com/overtone1000/uDAQC/blob/db2439e05dfa67c49c9bfb5320c6c1842942f3d9/Components/Device/src/Network/Network.cpp
    other:
      https://github.com/esp8266/Arduino/blame/c55f49bd6103dab81c6a389470f6d5bbbee399d0/boards.txt#L420-L471
      https://stackoverflow.com/questions/46289283/esp8266-captive-portal-with-pop-up
*/

#ifndef _AL_WIFICONNECTEVO_H_
#define _AL_WIFICONNECTEVO_H_

  #include "def.h"

  #include "wcevo_scan.h"
  #include "credential.h"
  #include "staconnect.h"
  #include "apconnect.h"
  #include "wcevo_wifimanager.h"

  static const char WCEVO_DEBUGREGION_WCEVO [] PROGMEM = "wcevo";
  static const char WCEVO_DEBUGREGION_AP    [] PROGMEM = "wcevo ap";
  static const char WCEVO_DEBUGREGION_STA   [] PROGMEM = "wcevo STA";

  const char * const WIFI_STA_STATUS[] PROGMEM
  {
    "WL_IDLE_STATUS",     // 0 STATION_IDLE
    "WL_NO_SSID_AVAIL",   // 1 STATION_NO_AP_FOUND
    "WL_SCAN_COMPLETED",  // 2
    "WL_CONNECTED",       // 3 STATION_GOT_IP
    "WL_CONNECT_FAILED",  // 4 STATION_CONNECT_FAIL, STATION_WRONG_PASSWORD(NI)
    "WL_CONNECTION_LOST", // 5
    "WL_DISCONNECTED",    // 6
    "WL_STATION_WRONG_PASSWORD" // 7 KLUDGE
  };
  const char S_NA[]                 PROGMEM = "Unknown";

  const char WCEVO_PTJSON_001[] PROGMEM = "credentials";
  const char WCEVO_PTJSON_002[] PROGMEM = "credential";
  const char WCEVO_PTJSON_003[] PROGMEM = "server";
  const char WCEVO_PTJSON_004[] PROGMEM = "hostname";
  const char WCEVO_PTJSON_005[] PROGMEM = "apssid";
  const char WCEVO_PTJSON_006[] PROGMEM = "appsk";
  const char WCEVO_PTJSON_007[] PROGMEM = "connection";
  const char WCEVO_PTJSON_008[] PROGMEM = "cm";
  const char WCEVO_PTJSON_009[] PROGMEM = "cmf";
  const char WCEVO_PTJSON_010[] PROGMEM = "cu";
  const char WCEVO_PTJSON_011[] PROGMEM = "scan";
  const char WCEVO_PTJSON_012[] PROGMEM = "mode_start";
  static const char* const WCEVO_PTJSON_ALL[] PROGMEM = {
    WCEVO_PTJSON_001, 
    WCEVO_PTJSON_002, 
    WCEVO_PTJSON_003, 
    WCEVO_PTJSON_004,
    WCEVO_PTJSON_005,
    WCEVO_PTJSON_006,
    WCEVO_PTJSON_007,
    WCEVO_PTJSON_008,
    WCEVO_PTJSON_009,
    WCEVO_PTJSON_010,
    WCEVO_PTJSON_011,
    WCEVO_PTJSON_012
  };

  /**
   * mod de connection a l'initialisation du programm 
   * STA
   *  STA -> si sta nn dispo ou si connection pas etblie -> reset
   * STAAP 
   *  STA -> si sta nn dispo ou si connection pas etblie -> AP
   * AP
   *  ap
   */
  typedef enum : uint8_t {
    WCEVO_CM_STA = 0,
    WCEVO_CM_AP,
    WCEVO_CM_STAAP,
    WCEVO_CM_NONE
  } wcevo_connectmod_t;
  const char* const wcevo_connectmod_s[] PROGMEM = { "CM_STA", "CM_AP", "CM_STAAP", "CM_NONE" };
  extern wcevo_connectmod_t wcevo_connectmodArray_t[];

  /**
   * ???
   */
  typedef enum : uint8_t {
    WCEVO_CF_RESET = 0,
    WCEVO_CF_NEXTAP,
    WCEVO_CF_AP,
    WCEVO_CF_CALLBACK,
    WCEVO_CF_NONE
  } wcevo_connectfail_t;
  const char* const wcevo_connectfail_s[] PROGMEM = { "CF_RESET", "CF_NEXTAP", "CF_AP", "WCEVO_CF_CALLBACK", "CF_NONE" };
  extern wcevo_connectfail_t wcevo_connectfaildArray_t[];


  typedef enum : uint8_t {
    WCEVO_UM_STA = 0,
    WCEVO_UM_STAAP,
    WCEVO_UM_AP,
    WCEVO_UM_UPDATED,
    WCEVO_UM_NONE
  } wcevo_updateMod_t;


  class WCEVO_manager
  {
    typedef std::function<void()> callback_function_t;

    wcevo_updateMod_t _updateMod = WCEVO_UM_NONE;

    const char * config_filepath = "/wcevo_config.json";

    AsyncWebServer  * _webserver ;
    DNSServer       * _dnsServer ;
    // AsyncUDP         * _udpMultiServer;
    // AsyncUDP         * _udpServer;

    WCEVO_server                * _server       = nullptr;
    LList<WCEVO_credential *>   _credentials;
    WCEVO_credential            * _credential   = nullptr;
    boolean                     _credentialUse  = false;

    WCEVO_scanNetwork           _scanNtwork;

    WCEVO_APconnect             _APCO;
    WCEVO_STAconnect            _STACO;
    WiFiManagerCPY              _WM;
//
    wcevo_connectmod_t  _CONNECTMOD   = wcevo_connectmod_t::WCEVO_CM_AP;  
    wcevo_connectfail_t _CONNECTFAIL  = wcevo_connectfail_t::WCEVO_CF_NEXTAP; 
    wcevo_connectfail_t _CONNECTFAIL_OLD  = wcevo_connectfail_t::WCEVO_CF_NEXTAP; 
    uint8_t _mode_start = 0;
    
    boolean _scanNetwork_gotSSID = false;

    boolean _otaSetup = false;

    boolean _scanNetwork_running = true;
    boolean _scanNetwork_requiered = false;

    callback_function_t _cb_serverEvent = nullptr;;
    callback_function_t _cb_webserverEvent = nullptr;;
    callback_function_t _cb_webserverOn = nullptr;;
    callback_function_t _cb_cfFail = nullptr;;
    // IPAddress _udp_ip    = {239, 0, 0, 57};
    // IPAddress _udpMulti_ip     = {239, 0, 0, 57};
    // uint16_t   _udpMulti_port  = 9200;
    // uint16_t   _udp_port   = 9200;

    unsigned long _configPortalTimeout    = 0;
    uint8_t _configPortalAuto             = 1;
    uint8_t _configPortalMod              = 0;
    unsigned long _configPortalStart      = 0;
    boolean configPortalHasTimeout();

  public:

    uint8_t get_mode_start() {return _mode_start;}
    void set_mode_start(uint8_t v) {_mode_start = v;}

    boolean updateModReady()  {return (_updateMod != WCEVO_UM_NONE && _updateMod != WCEVO_UM_UPDATED);}
    wcevo_updateMod_t get_updateMod() const {return _updateMod;}
    void set_updateModEnd()     {_updateMod = WCEVO_UM_UPDATED;}
    void set_updateModOff()   {_updateMod = WCEVO_UM_NONE;}

    uint8_t _credentialPos  = 0;

    // WCEVO_manager(WCEVO_server * cr, DNSServer*dnsServer,AsyncWebServer*webserver);
    WCEVO_manager(const char * const & v1 = NULL, const char * const & v2 = NULL, const char * const & v3 = NULL, DNSServer* v4 = nullptr,AsyncWebServer* v5 = nullptr);
    WCEVO_manager(const char * const & v1 = NULL, const char * const & v2 = NULL, DNSServer* v4 = nullptr,AsyncWebServer* v5 = nullptr);
    // WCEVO_manager(const char * const & v1 = NULL, DNSServer* v4 = nullptr,AsyncWebServer* v5 = nullptr);
    WCEVO_manager(const char * const & v1 = NULL, DNSServer* v4 = nullptr,AsyncWebServer* v5 = nullptr);
    WCEVO_manager(DNSServer* v4 = nullptr,AsyncWebServer* v5 = nullptr);
    ~WCEVO_manager();
    void init(const char * const &, const char * const &, const char * const &);
    
    WCEVO_server                * server();
    LList<WCEVO_credential *>   * credentials();
    WCEVO_credential            * credential();
    WCEVO_scanNetwork           * networkScan();
    WCEVO_STAconnect            * get_STA();
    WCEVO_APconnect             * get_AP();
    WiFiManagerCPY              * get_WM();

    callback_function_t _cb_webserveAprOn = nullptr;;
    callback_function_t _cb_webserveAprEvent = nullptr;;

  private:

    void credentials_delete();

    void set_credential(WCEVO_credential*&, JsonArray & arr_1);
    void set_credential(JsonArray & arr_1);
    void set_credential(WCEVO_credential*&);

    WCEVO_credential * credential_getBestSSID();


    void get_credential_json(WCEVO_credential*, JsonArray &);
    void get_credentials_json(JsonArray &);
    void get_credentials_json(WCEVO_credential*, DynamicJsonDocument &);
    void get_credentials_json(DynamicJsonDocument &);


    boolean   isConnected();    
    IPAddress localIP();
    void      sta_loop();
    void      sta_reconnect();
    uint8_t   sta_reconnect_end(boolean apSetup = true);
    uint8_t   sta_getMaxAettemp();

    boolean _cb_serverEvent_loaded = false;
  public:
    void set_cb_cfFail(callback_function_t f) { _cb_cfFail = std::move(f); };

    void set_cb_serverEvent(callback_function_t f) { _cb_serverEvent = std::move(f); };

    void set_cb_webserverOn(callback_function_t f) { _cb_webserverOn = std::move(f); };
    void set_cb_webserverEvent(callback_function_t f) { _cb_webserverEvent = std::move(f); };

    void set_cb_webserveAprOn(callback_function_t f) { _cb_webserveAprOn = std::move(f); };
    void set_cb_webserveAprEvent(callback_function_t f) { _cb_webserveAprEvent = std::move(f); };

    void setConfigPortalTimeout(unsigned long seconds);
    void setConfigPortalAuto(uint8_t v) { _configPortalAuto = v ; };

    boolean sta_connected();    

    void set_credentialUse(boolean);
    boolean get_credentialUse();

    wcevo_connectmod_t get_cm();
    wcevo_connectfail_t get_cmFail();
    void set_cm(wcevo_connectmod_t);
    void set_cmFail(wcevo_connectfail_t);

    boolean get_scanNetwork_running();
    boolean get_scanNetwork_requiered();
    void set_scanNetwork_running(boolean);
    void set_scanNetwork_requiered(boolean);

    void set_credential(uint8_t);
    void set_credential(const String &, const String &);
    void set_credentials(uint8_t, const String &, const String &);

    void get_credentials_html(String&);

    void credential_print();

    uint8_t credentials_add(const char * const &, const char * const & );
    void credentials_print();

    void mdns_setup();
    void ota_setup();

    void handleConnection();

    void start();
    void print();
    void keyboard_getter(const String &);
    void keyboard_print();
    void api_getter(DynamicJsonDocument & doc, const char *);
    void api_key(DynamicJsonDocument & doc, const String & arg);

    #ifdef FILESYSTEM
      void credentials_to_fs(
        boolean cu,
        wcevo_connectmod_t cm = WCEVO_CM_NONE, 
        wcevo_connectfail_t cmf = WCEVO_CF_NONE); 
      void credentials_to_fs(
        wcevo_connectmod_t cm = WCEVO_CM_NONE, 
        wcevo_connectfail_t cmf = WCEVO_CF_NONE); 
      
      void credentials_from_fs(); 
    #endif    
//

    /*
    void initSTA();
    void initAP();
    
    ;
    

    String getWLStatusString(uint8_t status);
    String getWLStatusString();
    */

/*
    bool wifiConnectNew(const char * ssid, const char * pass,bool connect);
    bool WiFi_Mode(WiFiMode_t m,bool persistent);
    bool WiFi_Mode(WiFiMode_t m);
    bool WiFi_enableSTA(bool enable,bool persistent);
    bool WiFi_enableSTA(bool enable);
*/

  };
  WCEVO_manager * WCEVO_managerPtrGet();


#endif // MAIN_H