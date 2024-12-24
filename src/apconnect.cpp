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

#include "../include/apconnect.h"
#include "../include/al_wificonnectevo.h"


#if (WCEVO_PORTAL == WCEVO_PORTAL_UI)
  const char *_customHeadElement    = "";
  const char *_customOptionsElement = "";  
#endif


void WCEVO_APconnect::init(WCEVO_server * server, DNSServer* dns, AsyncWebServer* web){
  _server = server; 
  _webserver = web;
  _dnsServer = dns;  
}
boolean WCEVO_APconnect::get_active(){return _active;}
void WCEVO_APconnect::set_active(boolean v1){_active = v1;}

void WCEVO_APconnect::shutdown(){
  _serverInitialized = false;
  _active = false;
  _webserver->reset();
  _dnsServer->stop(); //  free heap ?
  WiFi.softAPdisconnect(false); 
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Access point disabled.\n");
}

// void WCEVO_APconnect::loop(){
   // _dnsServer->processNextRequest();  
// }

void WCEVO_APconnect::setup_webserver(boolean webserverBegin) { 
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Init AP interfaces\n");

  if (WCEVO_managerPtrGet()->_cb_webserveAprEvent) {
    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "AP setup external event webserver handle\n");
    WCEVO_managerPtrGet()->_cb_webserveAprEvent();
  }

  #if (WCEVO_PORTAL == WCEVO_PORTAL_UI)
  _webserver->on(F_EX(R_root),          std::bind(&WCEVO_APconnect::handleRoot<SERVERREQUEST>,  this, std::placeholders::_1)).setFilter(ON_AP_FILTER);
  _webserver->on("/fwlink",             std::bind(&WCEVO_APconnect::handleRoot<SERVERREQUEST>,  this, std::placeholders::_1)).setFilter(ON_AP_FILTER); // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  _webserver->on("/generate_204",       std::bind(&WCEVO_APconnect::handleRoot<SERVERREQUEST>,  this, std::placeholders::_1)).setFilter(ON_AP_FILTER); // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  
  _webserver->on(F_EX(R_wifi),          std::bind(&WCEVO_APconnect::handleWifi,                 this, std::placeholders::_1, false  )).setFilter(ON_AP_FILTER);
  _webserver->on(F_EX(R_wifiMulti),     std::bind(&WCEVO_APconnect::handleWifiMulti,            this, std::placeholders::_1, false  )).setFilter(ON_AP_FILTER);
  
  _webserver->on(F_EX(R_wifiScan),      std::bind(&WCEVO_APconnect::handleWifi,                 this, std::placeholders::_1, true   )).setFilter(ON_AP_FILTER);
  _webserver->on(F_EX(R_wifiScanMulti), std::bind(&WCEVO_APconnect::handleWifiMulti,            this, std::placeholders::_1, true   )).setFilter(ON_AP_FILTER);

  _webserver->on(F_EX(R_wifisave),      std::bind(&WCEVO_APconnect::handleWifiSave,             this, std::placeholders::_1)).setFilter(ON_AP_FILTER);  
  _webserver->on(F_EX(R_wifisaveMulti), std::bind(&WCEVO_APconnect::handleWifiSaveMulti,        this, std::placeholders::_1)).setFilter(ON_AP_FILTER);  

  _webserver->on(F_EX(R_wifico),        std::bind(&WCEVO_APconnect::handleWifiCo,               this, std::placeholders::_1)).setFilter(ON_AP_FILTER);  
  _webserver->on(F_EX(R_info),          std::bind(&WCEVO_APconnect::handleInfo_v2,              this, std::placeholders::_1)).setFilter(ON_AP_FILTER);          
  _webserver->on(F_EX(R_infoAndroid),   std::bind(&WCEVO_APconnect::handleInfo_v1,              this, std::placeholders::_1)).setFilter(ON_AP_FILTER);          
  _webserver->on(F_EX(R_wifimod),       std::bind(&WCEVO_APconnect::handleWifiMod,              this, std::placeholders::_1)).setFilter(ON_AP_FILTER);
  _webserver->on(F_EX(R_wifisavmod),    std::bind(&WCEVO_APconnect::handleWifiSaveMod,          this, std::placeholders::_1)).setFilter(ON_AP_FILTER);  
  _webserver->on(F_EX(R_restart),       std::bind(&WCEVO_APconnect::handleReset,                this, std::placeholders::_1)).setFilter(ON_AP_FILTER);  
  #endif

#ifdef ALWC_WS_API
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "AP register HTTP_GET request : /wcapi\n"); 
  _webserver->on("/wcapi", HTTP_GET, [this](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(3500);
    for (unsigned int i = 0; i < request->args(); i++) {
      // message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
      Serial.printf_P(PSTR("argName: %s arg: %s\n"), request->argName(i).c_str(), request->arg(i).c_str());
      if (request->argName(i) == "api") {
        WCEVO_managerPtrGet()->api_key(doc, request->arg(i) );
      } 
      int rSize = 0;
      const char** split = al_tools::explode(request->arg(i), ',', rSize);
      if (split) {
        for(int j = 0; j < rSize; ++j) {
          Serial.printf_P(PSTR("[%d] %s\n"), j , split[j]);
          if (request->argName(i) == "wc")      WCEVO_managerPtrGet()->api_getter(doc, split[j]);                           
          #ifdef ALSI_ENABLED
          if (request->argName(i) == "alsi")    ALSYSINFO_getterByCat(doc, split[j]);                           
          if (request->argName(i) == "alsii")   ALSYSINFO_getterByKey(doc, split[j]);   
          #endif                          
        }
        for(int j = 0; j < rSize; ++j) {
          delete split[j];
        }
        delete[] split; 
      } else {
        if (request->argName(i) == "wc")      WCEVO_managerPtrGet()->api_getter(doc, request->arg(i).c_str());                           
        #ifdef ALSI_ENABLED
        if (request->argName(i) == "alsi")    ALSYSINFO_getterByCat(doc, request->arg(i).c_str());                           
        if (request->argName(i) == "alsii")   ALSYSINFO_getterByKey(doc, request->arg(i).c_str());   
        #endif           
      }       
    }    
    String result; 
    serializeJson(doc,result); 
    request->send(200, "application/json", result);
  }).setFilter(ON_AP_FILTER);    
#endif

  // typedef std::function<void()> getCb_t;
  // getCb_t getCb = WCEVO_managerPtrGet()->get_cb_webserveAprOn();
  // 
  if (WCEVO_managerPtrGet()->_cb_webserveAprOn) {
    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "AP setup external http webserver handle\n");
    WCEVO_managerPtrGet()->_cb_webserveAprOn();
  }
  
  if (webserverBegin) {
    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "AP register onNotFound\n"); 
    _webserver->onNotFound(std::bind(&WCEVO_APconnect::handleNotFound, this, std::placeholders::_1));

    ALT_TRACEC(WCEVO_DEBUGREGION_AP, "AP webserverBegin\n");
    _webserver->begin();
    _serverInitialized = true;
  }
  
}
void WCEVO_APconnect::setup(boolean isSTA, boolean setmod, boolean webserverBegin) {

  if(!WiFi.isConnected()){
    // this fixes most ap problems, however, simply doing mode(WIFI_AP) does not work if sta connection is hanging, must `wifi_station_disconnect` 
    WCEVO_managerPtrGet()->get_WM()->WiFi_Disconnect();
    WCEVO_managerPtrGet()->get_WM()->WiFi_enableSTA(false);
    ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Disabling STA\n");
  }
  else {
    // @todo even if sta is connected, it is possible that softap connections will fail, IOS says "invalid password", windows says "cannot connect to this network" researching
    WCEVO_managerPtrGet()->get_WM()->WiFi_enableSTA(true);
  }

  #ifdef ESP8266
    // @bug workaround for bug #4372 https://github.com/esp8266/Arduino/issues/4372
    if(!WiFi.enableAP(true)) {
      ALT_TRACEC(WCEVO_DEBUGREGION_AP , "[ERROR] enableAP failed!\n"); //DEBUG_ERROR
      return;
    }
  #endif

  // stack 
  yield();

  // if (disconnect) WiFi.disconnect(false);
  // if (setmod)     WiFi.mode(WIFI_AP); 

  const char * ssid;
  const char * pass;
  _server->get_apSSID(ssid);
  _server->get_apPsk(pass);
  Serial.printf_P(PSTR("\n\t>>> Opening access point - ssid: %s psk: %s - ip: 8.8.8.8, gateway/subnet: 255.255.255.0\n\n"), ssid, pass);
  // WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

  // use ip for android captive portal redirection (8,8,4,4 work to)  
  IPAddress _staticIP(8, 8, 8, 8);; 

  // defailt subnet
  IPAddress _subnet(255, 255, 255, 0);

  WiFi.softAPConfig(_staticIP, _staticIP, _subnet);

  // 
  WiFi.softAP(ssid, pass);

  // mdns - http-txp port 80
  WCEVO_managerPtrGet()->mdns_setup();

  // start dns for the portal redirection
  _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  if (!_dnsServer->start(53, "*", WiFi.softAPIP())) {
    Serial.println(F("Could not start Captive DNS Server!"));
  }  

  // register hhtp handle from ap ip
  setup_webserver(webserverBegin);  

  // done
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Set _AP.active to TRUE\n");
  _active = true; 
}



/*
  boolean WCEVO_APconnect::captivePortal(AsyncWebServerRequest *request)
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L3317-L3325
  https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L1330-L1342

  void WCEVO_APconnect::handleNotFound(AsyncWebServerRequest *request)
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L2288-L2310
  https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L1227-L1254
*/
// region ################################################ WIFIMANAGER CAPTIVPORTAL REDIRECT
boolean WCEVO_APconnect::captivePortal(AsyncWebServerRequest *request)
{
  if (!WCEVO_managerPtrGet()->get_WM()->isIp(request->host()))
  {
    // ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Request redirected to captive portal\n");
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
    response->addHeader("Location", String("http://") + WCEVO_managerPtrGet()->get_WM()->toStringIp(request->client()->localIP()));
    request->send(response);
    return true;
  }
  return false;
}

void WCEVO_APconnect::handleNotFound(AsyncWebServerRequest *request)
{
  // ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Handle not found\n");
  if (captivePortal(request))
  {
    // if captive portal redirect instead of displaying the error page
    return;
  }    

  String message = "File Not Found\n\n";
  message += F("URI: ");
  message += request->url();
  message += F("\nMethod: ");
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += request->args();
  message += F("\n");

  for (unsigned int i = 0; i < request->args(); i++)
  {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", message);
  response->addHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  response->addHeader(F("Pragma"), F("no-cache"));
  response->addHeader(F("Expires"), F("-1"));
  request->send(response);
}  
// endregion >>>> WIFIMANAGER CAPTIVPORTAL REDIRECT


#if WCEVO_PORTAL == WCEVO_PORTAL_UI
/*
  void WCEVO_APconnect::handleReset(AsyncWebServerRequest *request)
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L2231-L2250
  https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L1202-L1225
*/
// region ################################################ WIFIMANAGER HANDLE RESTET
void WCEVO_APconnect::handleReset(AsyncWebServerRequest *request) {
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Reset\n");
  String page = FPSTR(WCEVO_HTTP_HEAD);
  page.replace("{v}", "Reset");
  page += FPSTR(WCEVO_HTTP_SCRIPT);
  page += FPSTR(WCEVO_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WCEVO_HTTP_HEAD_END);
  page += F("Module will reset in a few seconds");
  page += FPSTR(WCEVO_HTTP_END);
  request->send(200, FPSTR(HTTP_HEAD_CT), page);
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Sent reset page");
  delay(1000);
  ESP.restart();
}  
// endregion >>>> WIFIMANAGER HANDLE RESET


/**
 * https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L3345-L3362
 * encode htmlentities
 * @since $dev
 * @param  string str  string to replace entities
 * @return string      encoded string
 */
String WCEVO_APconnect::htmlEntities(String str, bool whitespace) {
  str.replace("&","&amp;");
  str.replace("<","&lt;");
  str.replace(">","&gt;");
  if(whitespace) str.replace(" ","&#160;");
  // str.replace("-","&ndash;");
  // str.replace("\"","&quot;");
  // str.replace("/": "&#x2F;");
  // str.replace("`": "&#x60;");
  // str.replace("=": "&#x3D;");
return str;
}

/*
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L1988-L2211
*/
// region ################################################ WIFIMANAGER GETINFO
String WCEVO_APconnect::getInfoData(String id){

  String p;
  // @todo add WM versioning
  if(id==F("esphead"))p = FPSTR(HTTP_INFO_esphead);
  else if(id==F("wifihead"))p = FPSTR(HTTP_INFO_wifihead);
  else if(id==F("uptime")){
    // subject to rollover!
    p = FPSTR(HTTP_INFO_uptime);
    p.replace(FPSTR(T_1),(String)(millis() / 1000 / 60));
    p.replace(FPSTR(T_2),(String)((millis() / 1000) % 60));
  }
  else if(id==F("chipid")){
    p = FPSTR(HTTP_INFO_chipid);
    p.replace(FPSTR(T_1),String(WIFI_getChipId(),HEX));
  }
  #ifdef ESP32
  else if(id==F("chiprev")){
      p = FPSTR(HTTP_INFO_chiprev);
      String rev = (String)ESP.getChipRevision();
      #ifdef _SOC_EFUSE_REG_H_
        String revb = (String)(REG_READ(EFUSE_BLK0_RDATA3_REG) >> (EFUSE_RD_CHIP_VER_RESERVE_S)&&EFUSE_RD_CHIP_VER_RESERVE_V);
        p.replace(FPSTR(T_1),rev+"<br/>"+revb);
      #else
        p.replace(FPSTR(T_1),rev);
      #endif
  }
  #endif
  #ifdef ESP8266
  else if(id==F("fchipid")){
      p = FPSTR(HTTP_INFO_fchipid);
      p.replace(FPSTR(T_1),(String)ESP.getFlashChipId());
  }
  #endif
  else if(id==F("idesize")){
    p = FPSTR(HTTP_INFO_idesize);
    p.replace(FPSTR(T_1),(String)ESP.getFlashChipSize());
  }
  else if(id==F("flashsize")){
    #ifdef ESP8266
      p = FPSTR(HTTP_INFO_flashsize);
      p.replace(FPSTR(T_1),(String)ESP.getFlashChipRealSize());
    #elif defined ESP32
      p = FPSTR(HTTP_INFO_psrsize);
      p.replace(FPSTR(T_1),(String)ESP.getPsramSize());      
    #endif
  }
  else if(id==F("corever")){
    #ifdef ESP8266
      p = FPSTR(HTTP_INFO_corever);
      p.replace(FPSTR(T_1),(String)ESP.getCoreVersion());
    #endif      
  }
  #ifdef ESP8266
  else if(id==F("bootver")){
      p = FPSTR(HTTP_INFO_bootver);
      p.replace(FPSTR(T_1),(String)system_get_boot_version());
  }
  #endif  
  else if(id==F("cpufreq")){
    p = FPSTR(HTTP_INFO_cpufreq);
    p.replace(FPSTR(T_1),(String)ESP.getCpuFreqMHz());
  }
  else if(id==F("freeheap")){
    p = FPSTR(HTTP_INFO_freeheap);
    p.replace(FPSTR(T_1),(String)ESP.getFreeHeap());
  }
  else if(id==F("memsketch")){
    p = FPSTR(HTTP_INFO_memsketch);
    p.replace(FPSTR(T_1),(String)(ESP.getSketchSize()));
    p.replace(FPSTR(T_2),(String)(ESP.getSketchSize()+ESP.getFreeSketchSpace()));
  }
  else if(id==F("memsmeter")){
    p = FPSTR(HTTP_INFO_memsmeter);
    p.replace(FPSTR(T_1),(String)(ESP.getSketchSize()));
    p.replace(FPSTR(T_2),(String)(ESP.getSketchSize()+ESP.getFreeSketchSpace()));
  }
  else if(id==F("lastreset")){
    #ifdef ESP8266
      p = FPSTR(HTTP_INFO_lastreset);
      p.replace(FPSTR(T_1),(String)ESP.getResetReason());
    #elif defined(ESP32) && defined(_ROM_RTC_H_)
      // requires #include <rom/rtc.h>
      p = FPSTR(HTTP_INFO_lastreset);
      for(int i=0;i<2;i++){
        int reason = rtc_get_reset_reason(i);
        String tok = (String)T_ss+(String)(i+1)+(String)T_es;
        switch (reason)
        {
          //@todo move to array
          case 1  : p.replace(tok,F("Vbat power on reset"));break;
          case 3  : p.replace(tok,F("Software reset digital core"));break;
          case 4  : p.replace(tok,F("Legacy watch dog reset digital core"));break;
          case 5  : p.replace(tok,F("Deep Sleep reset digital core"));break;
          case 6  : p.replace(tok,F("Reset by SLC module, reset digital core"));break;
          case 7  : p.replace(tok,F("Timer Group0 Watch dog reset digital core"));break;
          case 8  : p.replace(tok,F("Timer Group1 Watch dog reset digital core"));break;
          case 9  : p.replace(tok,F("RTC Watch dog Reset digital core"));break;
          case 10 : p.replace(tok,F("Instrusion tested to reset CPU"));break;
          case 11 : p.replace(tok,F("Time Group reset CPU"));break;
          case 12 : p.replace(tok,F("Software reset CPU"));break;
          case 13 : p.replace(tok,F("RTC Watch dog Reset CPU"));break;
          case 14 : p.replace(tok,F("for APP CPU, reseted by PRO CPU"));break;
          case 15 : p.replace(tok,F("Reset when the vdd voltage is not stable"));break;
          case 16 : p.replace(tok,F("RTC Watch dog reset digital core and rtc module"));break;
          default : p.replace(tok,F("NO_MEAN"));
        }
      }
    #endif
  }
  else if(id==F("apip")){
    p = FPSTR(HTTP_INFO_apip);
    p.replace(FPSTR(T_1),WiFi.softAPIP().toString());
  }
  else if(id==F("apmac")){
    p = FPSTR(HTTP_INFO_apmac);
    p.replace(FPSTR(T_1),(String)WiFi.softAPmacAddress());
  }
  #ifdef ESP32
  else if(id==F("aphost")){
      p = FPSTR(HTTP_INFO_aphost);
      p.replace(FPSTR(T_1),WiFi.softAPgetHostname());
  }
  #endif
  #ifndef WM_NOSOFTAPSSID
  #ifdef ESP8266
  else if(id==F("apssid")){
    p = FPSTR(HTTP_INFO_apssid);
    p.replace(FPSTR(T_1),htmlEntities(WiFi.softAPSSID()));
  }
  #endif
  #endif
  else if(id==F("apbssid")){
    p = FPSTR(HTTP_INFO_apbssid);
    p.replace(FPSTR(T_1),(String)WiFi.BSSIDstr());
  }
  // softAPgetHostname // esp32
  // softAPSubnetCIDR
  // softAPNetworkID
  // softAPBroadcastIP

  else if(id==F("stassid")){
    p = FPSTR(HTTP_INFO_stassid);
    p.replace(FPSTR(T_1),htmlEntities((String)WCEVO_managerPtrGet()->get_WM()->WiFi_SSID(false)));
  }
  else if(id==F("staip")){
    p = FPSTR(HTTP_INFO_staip);
    p.replace(FPSTR(T_1),WiFi.localIP().toString());
  }
  else if(id==F("stagw")){
    p = FPSTR(HTTP_INFO_stagw);
    p.replace(FPSTR(T_1),WiFi.gatewayIP().toString());
  }
  else if(id==F("stasub")){
    p = FPSTR(HTTP_INFO_stasub);
    p.replace(FPSTR(T_1),WiFi.subnetMask().toString());
  }
  else if(id==F("dnss")){
    p = FPSTR(HTTP_INFO_dnss);
    p.replace(FPSTR(T_1),WiFi.dnsIP().toString());
  }
  else if(id==F("host")){
    p = FPSTR(HTTP_INFO_host);
    #ifdef ESP32
      p.replace(FPSTR(T_1),WiFi.getHostname());
    #else
    p.replace(FPSTR(T_1),WiFi.hostname());
    #endif
  }
  else if(id==F("stamac")){
    p = FPSTR(HTTP_INFO_stamac);
    p.replace(FPSTR(T_1),WiFi.macAddress());
  }
  else if(id==F("conx")){
    p = FPSTR(HTTP_INFO_conx);
    p.replace(FPSTR(T_1),WiFi.isConnected() ? FPSTR(S_y) : FPSTR(S_n));
  }
  #ifdef ESP8266
  else if(id==F("autoconx")){
    p = FPSTR(HTTP_INFO_autoconx);
    p.replace(FPSTR(T_1),WiFi.getAutoConnect() ? FPSTR(S_enable) : FPSTR(S_disable));
  }
  #endif
  #ifdef ESP32
  else if(id==F("temp")){
    // temperature is not calibrated, varying large offsets are present, use for relative temp changes only
    p = FPSTR(HTTP_INFO_temp);
    p.replace(FPSTR(T_1),(String)temperatureRead());
    p.replace(FPSTR(T_2),(String)((temperatureRead()+32)*1.8));
    // p.replace(FPSTR(T_3),(String)hallRead());
    p.replace(FPSTR(T_3),"NA");
  }
  #endif

  else if(id==F("aboutarduinover")){
    #ifdef VER_ARDUINO_STR
    p = FPSTR(HTTP_INFO_aboutarduino);
    p.replace(FPSTR(T_1),String(VER_ARDUINO_STR));
    #endif
  }
  // else if(id==F("aboutidfver")){
  //   #ifdef VER_IDF_STR
  //   p = FPSTR(HTTP_INFO_aboutidf);
  //   p.replace(FPSTR(T_1),String(VER_IDF_STR));
  //   #endif
  // }
  else if(id==F("aboutsdkver")){
    p = FPSTR(HTTP_INFO_sdkver);
    #ifdef ESP32
      p.replace(FPSTR(T_1),(String)esp_get_idf_version());
      // p.replace(FPSTR(T_1),(String)system_get_sdk_version()); // deprecated
    #else
    p.replace(FPSTR(T_1),(String)system_get_sdk_version());
    #endif
  }
  else if(id==F("aboutdate")){
    p = FPSTR(HTTP_INFO_aboutdate);
    p.replace(FPSTR(T_1),String(__DATE__) + " " + String(__TIME__));
  }
  return p;
}  
// endregion >>>> WIFIMANAGER GETINFO

/*
  void WCEVO_APconnect::handleInfo(AsyncWebServerRequest *request) 
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L1879-L1987
  https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L1173-L1200
*/
// region ################################################ WIFIMANAGER HANDLE INFO
void WCEVO_APconnect::handleInfo_v1(AsyncWebServerRequest *request) {
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Info\n");
  String page = FPSTR(WCEVO_HTTP_HEAD);
  page.replace("{v}", "Info");
  page += FPSTR(WCEVO_HTTP_SCRIPT);
  page += FPSTR(WCEVO_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WCEVO_HTTP_HEAD_END);
  page += F("<h2>/info</h2><hr>");
  page += F("<dl>");
  String s1;
  if (!WiFi.isConnected()) {
    s1 = FPSTR(HTTP_INFO_con_1);
    s1.replace(FPSTR(T_1), FPSTR(HTTP_INFO_ap));
    page += s1;
    s1 = FPSTR(HTTP_INFO_con_3);
    s1.replace(FPSTR(T_1),htmlEntities(WiFi.softAPSSID()));
    page += s1;    
    s1 = FPSTR(HTTP_INFO_con_2);
    s1.replace(FPSTR(T_1), WiFi.softAPIP().toString());
    page += "<br/>"; 
    page += s1;    
  } else {
    s1 = FPSTR(HTTP_INFO_con_1);
    s1.replace(FPSTR(T_1), FPSTR(HTTP_INFO_sta));
    page += s1;
    s1 = FPSTR(HTTP_INFO_con_3);
    s1.replace(FPSTR(T_1),htmlEntities((String)WCEVO_managerPtrGet()->get_WM()->WiFi_SSID(false)));
    page += s1;        
    s1 = FPSTR(HTTP_INFO_con_2);
    s1.replace(FPSTR(T_1), WiFi.localIP().toString());
    page += "<br/>"; 
    page += s1;  
  }
  // if (connect == true)
  // {
  //   page += F("<dt>Trying to connect</dt><dd>");
  //   page += WiFi.status();;
  //   page += F("</dd>");
  // }

  page += F("</dl>");
  page += FPSTR(HTTP_HOME_LINK);
  page += FPSTR(WCEVO_HTTP_END);
  request->send(200, FPSTR(HTTP_HEAD_CT), page);
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Sent info page\n");
}
void WCEVO_APconnect::handleInfo_v2(AsyncWebServerRequest *request) {
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Info\n");
  String page = FPSTR(WCEVO_HTTP_HEAD);
  page.replace("{v}", "Info");
  page += FPSTR(WCEVO_HTTP_SCRIPT);
  page += FPSTR(WCEVO_HTTP_STYLE);
  page += _customHeadElement;
  // if (connect == true)
  // {
    // page += F("<meta http-equiv=\"refresh\" content=\"5; url=/i\">");
  // }
  page += FPSTR(WCEVO_HTTP_HEAD_END);
  page += F("<h2>/info</h2><hr>");
  page += F("<dl>");
  // if (connect == true)
  // {
  //   page += F("<dt>Trying to connect</dt><dd>");
  //   page += WiFi.status();;
  //   page += F("</dd>");
  // }
  page += infoAsString();
  page += FPSTR(HTTP_HOME_LINK);
  page += FPSTR(WCEVO_HTTP_END);
  request->send(200, FPSTR(HTTP_HEAD_CT), page);
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Sent info page\n");
}
String WCEVO_APconnect::infoAsString() {
  String page;

  uint16_t infos = 0;

  //@todo convert to enum or refactor to strings
  //@todo wrap in build flag to remove all info code for memory saving
  #ifdef ESP8266
    infos = 28;
    String infoids[] = {
      F("esphead"),
      F("uptime"),
      F("chipid"),
      F("fchipid"),
      F("idesize"),
      F("flashsize"),
      F("corever"),
      F("bootver"),
      F("cpufreq"),
      F("freeheap"),
      F("memsketch"),
      F("memsmeter"),
      F("lastreset"),
      F("wifihead"),
      F("conx"),
      F("stassid"),
      F("staip"),
      F("stagw"),
      F("stasub"),
      F("dnss"),
      F("host"),
      F("stamac"),
      F("autoconx"),
      F("wifiaphead"),
      F("apssid"),
      F("apip"),
      F("apbssid"),
      F("apmac")
    };

  #elif defined(ESP32)
    // add esp_chip_info ?
    infos = 26;
    String infoids[] = {
      F("esphead"),
      F("uptime"),
      F("chipid"),
      F("chiprev"),
      F("idesize"),
      F("flashsize"),      
      F("cpufreq"),
      F("freeheap"),
      F("memsketch"),
      F("memsmeter"),      
      F("lastreset"),
      F("wifihead"),
      F("conx"),
      F("stassid"),
      F("staip"),
      F("stagw"),
      F("stasub"),
      F("dnss"),
      F("host"),
      F("stamac"),
      F("apssid"),
      F("wifiaphead"),
      F("apip"),
      F("apmac"),
      F("aphost"),
      F("apbssid")
      // F("temp")
    };
  #endif

  for(size_t i=0; i<infos;i++){
    if(infoids[i] != NULL) page += getInfoData(infoids[i]);
  }
  page += F("</dl>");

  page += F("<h3>About</h3><hr><dl>");
  page += getInfoData("aboutver");
  page += getInfoData("aboutarduinover");
  page += getInfoData("aboutidfver");
  page += getInfoData("aboutdate");
  page += F("</dl>");

  return page;
}  
// endregion >>>> WIFIMANAGER HANDLE INFO

/*
  void WCEVO_APconnect::handleRoot(AsyncWebServerRequest *request)
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L1269-L1293
  https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L884-L918
*/
// region ################################################ WIFIMANAGER HANDLE ROOT
template<class X>
void WCEVO_APconnect::handleRoot(X *request) {
  // AJS - maybe we should set a scan when we get to the root???
  // and only scan on demand? timer + on demand? plus a link to make it happen?
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Handle root\n");
  if (captivePortal(request))
  {
    // if captive portal redirect instead of displaying the page
    return;
  }
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "&c:1&s:\tSending Captive Portal\n");
  
  WCEVO_managerPtrGet()->networkScan()->list_clear();

  String page = FPSTR(WCEVO_HTTP_HEAD);
  page.replace("{v}", "Options");
  page += FPSTR(WCEVO_HTTP_SCRIPT);
  page += FPSTR(WCEVO_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WCEVO_HTTP_HEAD_END);
  page += F("<h1>");
  page += WCEVO_managerPtrGet()->server()->get_hostName();
  page += "</h1>";
  page += F("<h3>Menu</h3>");
  page += FPSTR(HTTP_PORTAL_OPTIONS);
  page += _customOptionsElement;
  page += FPSTR(WCEVO_HTTP_END);
  request->send(200, FPSTR(HTTP_HEAD_CT), page);
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "&c:1&s:\tSent...\n");
}  
// endregion >>>> WIFIMANAGER HANDLE ROOT

/*
  void WCEVO_APconnect::handleRoot(AsyncWebServerRequest *request)
  https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp#L1295-L1349
  https://github.com/alanswx/ESPAsyncWiFiManager/blob/master/src/ESPAsyncWiFiManager.cpp#L920-L1043
*/
// region ################################################ WIFIMANAGER HANDLE WIFI
void WCEVO_APconnect::handleWifi(AsyncWebServerRequest *request, boolean scan) {
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Handle wifi\n");
  String page = FPSTR(WCEVO_HTTP_HEAD);
  page.replace("{v}", "Config ESP");
  page += FPSTR(WCEVO_HTTP_SCRIPT);
  page += FPSTR(WCEVO_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WCEVO_HTTP_HEAD_END);
  page += WCEVO_managerPtrGet()->networkScan()->networkListAsString(scan);
  page += F("<br/>");
  page += FPSTR(HTTP_FORM_START);
  page += F("<br/>");
  page += FPSTR(HTTP_FORM_END);
  page += F("<br/>");
  String bt = FPSTR(HTTP_BT);
  bt.replace("{i}", "wc");
  bt.replace("{n}", "Connection");
  page += bt;
  page += F("<br/>");
  page += FPSTR(HTTP_SCAN_LINK);
  page += FPSTR(HTTP_HOME_LINK);
  page += FPSTR(WCEVO_HTTP_END);
  request->send(200, FPSTR(HTTP_HEAD_CT), page);
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "&c:1&s:\tSent config page\n");
}
void WCEVO_APconnect::handleWifiMulti(AsyncWebServerRequest *request, boolean scan) {
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "Handle wifi\n");
  String page = FPSTR(WCEVO_HTTP_HEAD);
  page.replace("{v}", "Config ESP");
  page += FPSTR(WCEVO_HTTP_SCRIPT);
  page += FPSTR(WCEVO_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WCEVO_HTTP_HEAD_END);
  page += WCEVO_managerPtrGet()->networkScan()->networkListAsString(scan);
  page += F("<br/>");
  page += F("<label style=\"font-weight: 700;\">Credentials from filesystem</label><br/><br/>");
  String credentials = "";
  WCEVO_managerPtrGet()->get_credentials_html(credentials);
  page += credentials;
  page += F("<br/>");
  page += FPSTR(HTTP_FORMMULTI_START);
  page += F("<br/>");
  page += FPSTR(HTTP_FORM_END);
  page += F("<br/>");
  // String bt = FPSTR(HTTP_BT);
  // bt.replace("{i}", "wc");
  // bt.replace("{n}", "Connection");
  // page += bt;
  // page += F("<br/>");
  page += FPSTR(HTTP_SCAN_LINKMULTI);
  page += FPSTR(HTTP_HOME_LINK);
  page += FPSTR(WCEVO_HTTP_END);
  request->send(200, FPSTR(HTTP_HEAD_CT), page);
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "&c:1&s:\tSent config page\n");
}
// endregion >>>> WIFIMANAGER HANDLE WIFI


void WCEVO_APconnect::handleWifiMod(AsyncWebServerRequest *request) {
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "handleWifiMod\n");
  String page = FPSTR(WCEVO_HTTP_HEAD);
  page.replace("{v}", "Config ESP");
  page += FPSTR(WCEVO_HTTP_SCRIPT);
  page += FPSTR(WCEVO_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WCEVO_HTTP_HEAD_END);
  page += FPSTR(HTTP_FORM_CM_START);
  page += F("<h2>/Server setup</h2><hr>");
  page += F("<p>");
  page += F("Station type");
  page += F("<p/>");
  String radio = FPSTR(HTTP_RADIO);
  radio.replace(F("{i}"), F("STA"));
  radio.replace(F("{n}"), F("connect_mod"));
  radio.replace(F("{v}"), F("0"));
  radio.replace(F("{1}"), F(" Station"));
  radio.replace(F("{c}"), (WCEVO_managerPtrGet()->get_cm() == WCEVO_CM_STA)?"checked":"");
  page += radio;
  radio = FPSTR(HTTP_RADIO);
  radio.replace(F("{i}"), F("STAAP"));
  radio.replace(F("{n}"), F("connect_mod"));
  radio.replace(F("{v}"), F("2"));
  radio.replace(F("{1}"), F(" Station + acces point"));
  radio.replace(F("{c}"), (WCEVO_managerPtrGet()->get_cm() == WCEVO_CM_STAAP)?"checked":"");
  page += radio;
  radio = FPSTR(HTTP_RADIO);
  radio.replace(F("{i}"), F("AP"));
  radio.replace(F("{n}"), F("connect_mod"));
  radio.replace(F("{v}"), F("1"));
  radio.replace(F("{1}"), F(" acces point"));
  radio.replace(F("{c}"), (WCEVO_managerPtrGet()->get_cm() == WCEVO_CM_AP)?"checked":"");
  page += radio;
  page += F("<br/>");
  page += F("<p>");
  page += F("Type d'identifiant");
  page += F("<p/>");  
  radio = FPSTR(HTTP_RADIO);
  radio.replace(F("{i}"), F("cred_simple"));
  radio.replace(F("{n}"), F("cred"));
  radio.replace(F("{v}"), F("1"));
  radio.replace(F("{1}"), F(" Credential"));
  radio.replace(F("{c}"), (WCEVO_managerPtrGet()->get_credentialUse())?"checked":"");
  page += radio;
  radio = FPSTR(HTTP_RADIO);
  radio.replace(F("{i}"), F("cred_multiple"));
  radio.replace(F("{n}"), F("cred"));
  radio.replace(F("{v}"), F("0"));
  radio.replace(F("{1}"), F(" Credential multiple"));
  radio.replace(F("{c}"), (!WCEVO_managerPtrGet()->get_credentialUse())?"checked":"");
  page += radio;
  page += F("<br/>");
  page += F("<p>");
  page += F("Start");
  page += F("<p/>");  
  radio = FPSTR(HTTP_RADIO);
  radio.replace(F("{i}"), F("mode_start_on"));
  radio.replace(F("{n}"), F("modestart"));
  radio.replace(F("{v}"), F("1"));
  radio.replace(F("{1}"), F(" Mode start on"));
  radio.replace(F("{c}"), (WCEVO_managerPtrGet()->get_mode_start())?"checked":"");
  page += radio;
  radio = FPSTR(HTTP_RADIO);
  radio.replace(F("{i}"), F("mode_start_off"));
  radio.replace(F("{n}"), F("modestart"));
  radio.replace(F("{v}"), F("1"));
  radio.replace(F("{1}"), F(" Mode start off"));
  radio.replace(F("{c}"), (!WCEVO_managerPtrGet()->get_mode_start())?"checked":"");
  page += radio;

  page += F("<br/>");
  page += FPSTR(HTTP_FORM_CM_END);
  page += FPSTR(HTTP_HOME_LINK);
  page += FPSTR(WCEVO_HTTP_END);
  request->send(200, FPSTR(HTTP_HEAD_CT), page);
  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "&c:1&s:\tSent config page\n");
}

void WCEVO_APconnect::handleWifiSaveMod(AsyncWebServerRequest *request) {
  if (request->hasArg(F("modestart"))) {
    uint8_t v = (request->arg(F("modestart"))=="0")?0:1;
    WCEVO_managerPtrGet()->set_mode_start(v); 
  }  
  if(request->hasArg(F("connect_mod"))) { 
    uint8_t p = request->arg(F("connect_mod")).toInt();
    WCEVO_managerPtrGet()->set_cm(wcevo_connectmodArray_t[p]);
  }
  if (request->hasArg(F("cred"))) {
    boolean cred = (request->arg(F("cred"))=="0")?false:true;
    WCEVO_managerPtrGet()->set_credentialUse(cred); 
  }
  WCEVO_managerPtrGet()->start();
  WCEVO_managerPtrGet()->print();
  #ifdef FILESYSTEM
  WCEVO_managerPtrGet()->credentials_to_fs();  
  #endif
  request->redirect(FPSTR(R_wifimod));
}
void WCEVO_APconnect::handleWifiCo(AsyncWebServerRequest *request) {

  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "handleWifiCo\n");


  /*
    !!!!!!!!
      si sta connect et new ssid = current ssid
        FAIL???????????????????????????
  */
  // WCEVO_managerPtrGet()->credentials_to_fs();
  // >>> Default settings for connection modes.
  WCEVO_managerPtrGet()->set_cm(WCEVO_CM_STAAP);
  WCEVO_managerPtrGet()->set_cmFail(WCEVO_CF_AP);
  WCEVO_managerPtrGet()->set_credentialUse(true);
  WCEVO_managerPtrGet()->start();
  // <<<
  // >>> Registration of credential(s) and configuration in the file system.
  #ifdef FILESYSTEM
  WCEVO_managerPtrGet()->credentials_to_fs();  
  #endif
  // <<<
  // >>> Debug serial print
  WCEVO_managerPtrGet()->credentials_print();
  WCEVO_managerPtrGet()->credential_print(); 
  WCEVO_managerPtrGet()->print(); 
  // <<<
  // >>> Preparing for Wi-Fi Router Connection Attempt 
  WCEVO_managerPtrGet()->get_STA()->reset();
  shutdown();
  // <<<

  String page = FPSTR(WCEVO_HTTP_HEAD);
  page.replace("{v}", "Exit");
  page += FPSTR(WCEVO_HTTP_SCRIPT);
  page += FPSTR(WCEVO_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WCEVO_HTTP_HEAD_END);
  page += FPSTR(WCEVO_HTTP_END);
  AsyncWebServerResponse *response = request->beginResponse(200, FPSTR(HTTP_HEAD_CT), page);
  response->addHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  request->send(response);

  // <<<
} 
void WCEVO_APconnect::handleWifiSaveMulti(AsyncWebServerRequest *request) {

  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "WiFi save\n");

  // Update of the identifiers.
  if ((request->arg(F("s")) != "") && (request->arg(F("p")) != "") && (request->arg(F("c")) != "")) {
    uint8_t p = request->arg(F("c")).toInt();
    WCEVO_managerPtrGet()->set_credentials(p, request->arg(F("s")), request->arg(F("p")) );

    // WCEVO_managerPtrGet()->set_credential(request->arg(F("s")), request->arg(F("p")));
  }

  // Debug serial print
  WCEVO_managerPtrGet()->credentials_print();
  WCEVO_managerPtrGet()->credential_print(); 

  // Registration of credential(s) and configuration in the file system.
  #ifdef FILESYSTEM
  WCEVO_managerPtrGet()->credentials_to_fs();  
  #endif

  request->redirect(FPSTR(R_wifiMulti));
}  
void WCEVO_APconnect::handleWifiSave(AsyncWebServerRequest *request) {

  ALT_TRACEC(WCEVO_DEBUGREGION_AP, "WiFi save\n");

  // Update of the identifiers.
  if ((request->arg(F("s")) != "") && (request->arg(F("p")) != ""))
    WCEVO_managerPtrGet()->set_credential(request->arg(F("s")), request->arg(F("p")));

  // Debug serial print
  WCEVO_managerPtrGet()->credentials_print();
  WCEVO_managerPtrGet()->credential_print(); 

  // Registration of credential(s) and configuration in the file system.
  #ifdef FILESYSTEM
  WCEVO_managerPtrGet()->credentials_to_fs();  
  #endif

  request->redirect(FPSTR(R_wifi));
}  

#endif
