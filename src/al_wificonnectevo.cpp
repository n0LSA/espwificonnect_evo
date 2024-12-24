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
      - multiple credential with sorted ssid-rssi
      - STA + AP || STA || AP
      - captive portal 
        - station mod
        - credential || multiple credential 
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
// ALWC_WS_OTA
// ALWC_WS_API

#include "../include/al_wificonnectevo.h" 
#include <altoolslib.h>

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
  
WCEVO_credential * _temp_WCEVO_credential = nullptr;

#define WIFISCAN_SSIDFILTERLEN  5

    wcevo_connectmod_t wcevo_connectmodArray_t[] = {
    WCEVO_CM_STA,
    WCEVO_CM_AP,
    WCEVO_CM_STAAP,
    WCEVO_CM_NONE
    };
    wcevo_connectfail_t wcevo_connectfaildArray_t[] = {
    WCEVO_CF_RESET,
    WCEVO_CF_NEXTAP,
    WCEVO_CF_AP,
    WCEVO_CF_CALLBACK,
    WCEVO_CF_NONE
    };

WCEVO_manager * WCEVO_managerPtr = nullptr;
WCEVO_manager * WCEVO_managerPtrGet(){return WCEVO_managerPtr;}
void WCEVO_manager::init(const char * const & Host, const char * const & APpass, const char * const & OTApass){
  #ifdef DEBUG_KEYBOARD
    _Sr_menu.add((const char *)"wifi_api",    (const char *)"r", [&]() { keyboard_print(); });
    _Sr_menu.add((const char *)"wifi_getter", (const char *)"_", [&](const String & v1, const String & v2) {  
      Serial.printf_P(PSTR("CMD: %s - VAL: %s\n"), v1.c_str(), v2.c_str());
      keyboard_getter(v1);    
    }, SR_MM::SRMM_KEYVAL);  
  #endif 
  _temp_WCEVO_credential = new WCEVO_credential((const char *)"", (const char *)"");
  _server = new WCEVO_server(Host,APpass,OTApass);  
  _APCO.init(_server, _dnsServer, _webserver);
  _STACO.init(_server);   
} 
WCEVO_manager::WCEVO_manager( const char * const & Host, const char * const & APpass, const char * const & OTApass, 
  DNSServer*dnsServer,AsyncWebServer*webserver) : _webserver(webserver), _dnsServer(dnsServer) {
  WCEVO_managerPtr = this;
  init(Host,APpass,OTApass);  
}
WCEVO_manager::WCEVO_manager( const char * const & Host, const char * const & APpass, 
  DNSServer*dnsServer,AsyncWebServer*webserver) : _webserver(webserver), _dnsServer(dnsServer) {
  WCEVO_managerPtr = this;
  init(Host,APpass,"wcevo_1234");  
}
WCEVO_manager::WCEVO_manager( const char * const & Host, DNSServer*dnsServer, AsyncWebServer*webserver) :
 _webserver(webserver), _dnsServer(dnsServer) {
  WCEVO_managerPtr = this;
  init(Host,"","wcevo_1234");  
}
WCEVO_manager::WCEVO_manager( DNSServer*dnsServer, AsyncWebServer*webserver) : 
  _webserver(webserver), _dnsServer(dnsServer) {
  WCEVO_managerPtr = this;
  init((const char *)"wcevo",(const char *)"",(const char *)"wcevo_1234");  
}

// WCEVO_manager::WCEVO_manager(WCEVO_server * ptr, DNSServer*dnsServer,AsyncWebServer*webserver){
//  WCEVO_managerPtr = this;
//  _server = new WCEVO_server(ptr);
//   _temp_WCEVO_credential = new WCEVO_credential("", "");
//   _APCO.init(_server, dnsServer, webserver);
//   _STACO.init(_server);
// }
WCEVO_manager::~WCEVO_manager(){
}

WCEVO_server * WCEVO_manager::server(){
  return _server;
}
WCEVO_scanNetwork * WCEVO_manager::networkScan(){
  return &_scanNtwork;
}
WCEVO_credential * WCEVO_manager::credential(){

  return (!_credential)?_temp_WCEVO_credential:_credential;}

void WCEVO_manager::start(){

  if (_mode_start == 1 && _CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_AP) {
    setConfigPortalTimeout(180);
    _CONNECTMOD = wcevo_connectmod_t::WCEVO_CM_AP;
  }

  setConfigPortalTimeout(180);

  uint8_t credsSize = _credentials.size();

  /*ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "cred size: %d\n", credsSize);*/
  /*ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "_scanNetwork_running: %d\n", _scanNetwork_running);*/

  if ((credsSize <= 0) && !_temp_WCEVO_credential) {
    _CONNECTMOD = wcevo_connectmod_t::WCEVO_CM_AP;
  }
  if (!_temp_WCEVO_credential && _credentialUse) _credentialUse = false;
  if (credsSize < 2) {
    if (credsSize > 0) set_credential(0);
    if ((credsSize <= 0)  && !_credentialUse && _temp_WCEVO_credential) _credentialUse =true;
    _CONNECTFAIL = wcevo_connectfail_t::WCEVO_CF_AP;
    _scanNetwork_running = false;
  }
  else {
    _CONNECTFAIL = wcevo_connectfail_t::WCEVO_CF_NEXTAP;
    if (_temp_WCEVO_credential && _credentialUse) _scanNetwork_running = false;
    else _scanNetwork_running = true;
  }
  // if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_NEXTAP && credsSize < 2) {
  //   _CONNECTFAIL = wcevo_connectfail_t::WCEVO_CF_AP;
  // }  
}
void WCEVO_manager::print(){
  uint8_t p = (uint8_t)_CONNECTMOD;
  String cm = FPSTR(wcevo_connectmod_s[p]);
  p = (uint8_t)_CONNECTFAIL;
  String cmf = FPSTR(wcevo_connectfail_s[p]);
  Serial.printf_P(PSTR("[WCEVO_manager::print]\n\tcm: %s\n\tcmf: %s\n\tcu: %d\n\tscan_runnig: %d\n\tscan_requiered: %d\n"),
    cm.c_str(),
    cmf.c_str(),
    _credentialUse,
    _scanNetwork_running,
    _scanNetwork_requiered);
  _server->print();
  credential_print();
  credentials_print();  

}
void WCEVO_manager::set_credential(WCEVO_credential*&ptr){
  _credential=ptr;
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(_credential->get_ssid().c_str(), _credential->get_psk().c_str());
}
void WCEVO_manager::set_credential(uint8_t pos){
  _credential = _credentials.get(pos);
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(_credential->get_ssid().c_str(), _credential->get_psk().c_str());  
}
void WCEVO_manager::set_credential(WCEVO_credential *& ptr, JsonArray & arr_1){
  String ssid = arr_1[0].as<String>();
  String psk  = arr_1[1].as<String>();
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(ssid.c_str(), psk.c_str());
  ptr = _temp_WCEVO_credential;
}
void WCEVO_manager::set_credential(JsonArray & arr_1){
  String ssid = arr_1[0].as<String>();
  String psk  = arr_1[1].as<String>();
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(ssid.c_str(), psk.c_str());
  _credential = _temp_WCEVO_credential;
}
void WCEVO_manager::set_credential(const String & ssid,const String & psk){
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(ssid.c_str(), psk.c_str());
  _credential = _temp_WCEVO_credential;
}

void WCEVO_manager::set_credentials(uint8_t p, const String & ssid, const String & psk){
  if (p >= _credentials.size()) return;
  WCEVO_credential * ptr = _credentials.get(p);
  char b_ssid[ssid.length()+1];
  char b_psk[psk.length()+1];
  sprintf(b_ssid, "%s", ssid.c_str());
  sprintf(b_psk, "%s", psk.c_str());
  ptr->set_psk(b_ssid, b_psk);
}


uint8_t WCEVO_manager::credentials_add(const char * const & v1, const char * const & v2){
  _credentials.add( new WCEVO_credential(v1, v2) ); 
  return _credentials.size()-1;
}
void WCEVO_manager::credentials_delete(){
  if (_credentials.size() <= 0) return;
  while (_credentials.size()) {
    WCEVO_credential *ptr = _credentials.shift();
    delete ptr;
  }
  _credentials.clear();
}

void WCEVO_manager::credential_print(){
  if (!_credential) _credential = _temp_WCEVO_credential;
  if (!_credential) {
    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "credential not instanced\n");
    return;
  }
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "-\n");
  _credential->print();
}

WCEVO_credential * WCEVO_manager::credential_getBestSSID(){
  if (_credentials.size() <= 0) {_scanNetwork_gotSSID = true; return nullptr;}
  int pos = networkScan()->get_bestSSID();
  if (pos == -1) {_scanNetwork_gotSSID = true; return nullptr;}
  // Serial.printf(" >>> IS TESTED: %d/%d = %d\n", pos, _credentials.size(), _credentials.get(pos)->get_tested());
  _credentialPos = pos; 
  return _credentials.get(pos);
}



void WCEVO_manager::get_credentials_html(String & result){
  const char * ssid;
  const char * psk;  
  for(uint8_t i = 0; i < _credentials.size(); ++i) {
    WCEVO_credential * item = _credentials.get(i);
    item->get_ssid(ssid);
    item->get_psk(psk);
    result += "<dt>"+al_tools::ch_toString(ssid)+"</dt><dd>"+al_tools::ch_toString(psk)+"</dd>";
  }
}

// FPSTR(WCEVO_PTJSON_002)
void WCEVO_manager::get_credential_json(WCEVO_credential * ptr, JsonArray & arr_1){
  if (!ptr) return;
  const char * ssid;
  const char * psk;
  ptr->get_ssid(ssid);
  ptr->get_psk(psk);
  JsonArray arr_2 = arr_1.createNestedArray();
  arr_2.add(ssid);    
  arr_2.add(psk);      
}

// FPSTR(WCEVO_PTJSON_001)
void WCEVO_manager::get_credentials_json(JsonArray & arr_1){
  for(int i = 0; i < _credentials.size(); ++i) {
    WCEVO_credential * item = _credentials.get(i);
    get_credential_json(item, arr_1);
  }
}

void WCEVO_manager::get_credentials_json(WCEVO_credential * ptr, DynamicJsonDocument & doc){
  JsonArray arr_1 = doc.createNestedArray(FPSTR(WCEVO_PTJSON_001));  
  get_credentials_json(arr_1);
  arr_1 = doc.createNestedArray(FPSTR(WCEVO_PTJSON_002));  
  get_credential_json(ptr, arr_1);
} 
void WCEVO_manager::get_credentials_json(DynamicJsonDocument & doc){
  get_credentials_json((_credential)?_credential:_temp_WCEVO_credential, doc);
} 

void WCEVO_manager::api_getter(DynamicJsonDocument & doc, const char * in) {
  char    * key = nullptr;
  boolean cpy   = true;
  uint8_t count = ARRAY_SIZE(WCEVO_PTJSON_ALL);
  if(isDigit(al_tools::ch_toString(in).charAt(0))) {
    for(int i = 0; i < count; ++i) {
      if (al_tools::ch_toString(in).toInt() == i) {
        key = new char[255];
        strcpy(key, WCEVO_PTJSON_ALL[i]);
        cpy=false;
        break;
      }  
    }    
  }
  if (cpy) {
    key = new char[strlen(in)+1];
    strcpy(key, in);
  }

  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_001) ) {
    JsonArray arr_1 = doc.createNestedArray(FPSTR(WCEVO_PTJSON_001)); 
    get_credentials_json(arr_1);
  }
  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_002)) {
    JsonArray arr_1 = doc.createNestedArray(FPSTR(WCEVO_PTJSON_002)); 
    get_credential_json(_temp_WCEVO_credential, arr_1);
  }  
  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_011)) {
    JsonArray arr_1 = doc.createNestedArray(FPSTR(WCEVO_PTJSON_011)); 
    networkScan()->networkListAsJson(2, arr_1);
  }  

  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_003) ) {
    JsonObject obj = doc.createNestedObject(FPSTR(WCEVO_PTJSON_003));
    const char * hostname;
    const char * apssid;
    const char * appsk;
    server()->get_hostName(hostname);
    server()->get_apSSID(apssid);
    server()->get_apPsk(appsk);
    obj[FPSTR(WCEVO_PTJSON_004)] = hostname;
    obj[FPSTR(WCEVO_PTJSON_005)] = apssid;
    obj[FPSTR(WCEVO_PTJSON_006)] = appsk;
  }  
  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_004) ) {
    if (!doc.containsKey(FPSTR(WCEVO_PTJSON_003))) doc.createNestedObject(FPSTR(WCEVO_PTJSON_003));
    if (!doc[FPSTR(WCEVO_PTJSON_004)][FPSTR(WCEVO_PTJSON_004)]) { 
      const char * val;
      server()->get_hostName(val);
      doc[FPSTR(WCEVO_PTJSON_003)][FPSTR(WCEVO_PTJSON_004)] = val;          
    } 
  }  
  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_005) ) {
    if (!doc.containsKey(FPSTR(WCEVO_PTJSON_003))) doc.createNestedObject(FPSTR(WCEVO_PTJSON_003));
    if (!doc[FPSTR(WCEVO_PTJSON_003)][FPSTR(WCEVO_PTJSON_005)]) { 
      const char * val;
      server()->get_apSSID(val);
      doc[FPSTR(WCEVO_PTJSON_003)][FPSTR(WCEVO_PTJSON_005)] = val;          
    } 
  } 
  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_006) ) {
    if (!doc.containsKey(FPSTR(WCEVO_PTJSON_003))) doc.createNestedObject(FPSTR(WCEVO_PTJSON_003));
    if (!doc[FPSTR(WCEVO_PTJSON_003)][FPSTR(WCEVO_PTJSON_006)]) { 
      const char * val;
      server()->get_apPsk(val);
      doc[FPSTR(WCEVO_PTJSON_003)][FPSTR(WCEVO_PTJSON_006)] = val;          
    } 
  }  

  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_007) ) {
    if (!doc.containsKey(FPSTR(WCEVO_PTJSON_007))) doc.createNestedObject(FPSTR(WCEVO_PTJSON_007));
    if (!doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_008)]) { 
      doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_008)] = wcevo_connectmod_s[_CONNECTMOD];          
    } 
    if (!doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_009)]) { 
      doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_009)] = wcevo_connectfail_s[_CONNECTFAIL];          
    } 
    if (!doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_010)]) { 
      doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_010)] = _credentialUse;          
    }         
  }   
  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_008) ) {
    if (!doc.containsKey(FPSTR(WCEVO_PTJSON_007))) doc.createNestedObject(FPSTR(WCEVO_PTJSON_007));
    if (!doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_008)]) { 
      doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_008)] = wcevo_connectmod_s[_CONNECTMOD];          
    } 
  }    
  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_009) ) {
    if (!doc.containsKey(FPSTR(WCEVO_PTJSON_007))) doc.createNestedObject(FPSTR(WCEVO_PTJSON_007));
    if (!doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_009)]) { 
      doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_009)] = wcevo_connectfail_s[_CONNECTFAIL];          
    } 
  } 
  if ( al_tools::ch_toString(key) == FPSTR(WCEVO_PTJSON_010) ) {
    if (!doc.containsKey(FPSTR(WCEVO_PTJSON_007))) doc.createNestedObject(FPSTR(WCEVO_PTJSON_007));
    if (!doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_010)]) { 
      doc[FPSTR(WCEVO_PTJSON_007)][FPSTR(WCEVO_PTJSON_010)] = _credentialUse;          
    } 
  }    
  
  if (key) delete key; 
}
void WCEVO_manager::keyboard_getter(const String & v1) {
  int rSize = 0;
  DynamicJsonDocument doc(3500);

  LList<SplitItem *> _SplitItem;
  splitText(v1, "&",  ':', &_SplitItem);

  for(int j = 0; j < _SplitItem.size(); ++j) {
    const char** split = al_tools::explode(_SplitItem[j]->_value, ',', rSize);
    if (split) {
      for(int i = 0; i < rSize; ++i) {
        Serial.printf_P(PSTR("[%d] %s\n"), i , split[i]);
        if (strcmp_P(_SplitItem[j]->_cmd, "wc") == 0)     api_getter(doc, split[i]);                           
        #ifdef ALSI_ENABLED
        if (strcmp_P(_SplitItem[j]->_cmd, "alsi") == 0)   ALSYSINFO_getterByCat(doc, split[i]);                           
        if (strcmp_P(_SplitItem[j]->_cmd, "alsii") == 0)  ALSYSINFO_getterByKey(doc, split[i]);   
        #endif                          
      }
      for(int i = 0; i < rSize; ++i) {
        delete split[i];
      }
      delete[] split; 
    } else {
        if (strcmp_P(_SplitItem[j]->_cmd, "wc") == 0)     api_getter(doc, _SplitItem[j]->_value);                           
        #ifdef ALSI_ENABLED
        if (strcmp_P(_SplitItem[j]->_cmd, "alsi") == 0)   ALSYSINFO_getterByCat(doc, _SplitItem[j]->_value);                           
        if (strcmp_P(_SplitItem[j]->_cmd, "alsii") == 0)  ALSYSINFO_getterByKey(doc, _SplitItem[j]->_value);   
        #endif        
    }
  }
  while (_SplitItem.size()) {
    SplitItem *eff = _SplitItem.shift();
    delete eff;
  }
  _SplitItem.clear();

  serializeJsonPretty(doc,Serial);Serial.println(); 
   
}

/*

{api: {wc: car: ["pos", "name"], ..}} wihout subkey
{api: {wc: car: ["pos", "name"], ["pos", "name", [["pos", "name"], ..], ... ]}} withsubkey

*/


void WCEVO_manager::api_key(DynamicJsonDocument & doc, const String & arg) {
  bool pr_wc    = false;
  bool pr_wcF   = false;
  #ifdef ALSI_ENABLED
  bool pr_alsi  = false;
  bool pr_alsiF = false;  
  #endif

  int rSize = 0;
  const char** split = al_tools::explode(arg, ',', rSize);
  if (split) {
    for(int j = 0; j < rSize; ++j) {
      if (al_tools::ch_toString(split[j]) == "wc")     pr_wc     = true;               
      if (al_tools::ch_toString(split[j]) == "wci")    pr_wcF    = true; 
      #ifdef ALSI_ENABLED
      if (al_tools::ch_toString(split[j]) == "alsi")   pr_alsi   = true;               
      if (al_tools::ch_toString(split[j]) == "alsii")  pr_alsiF  = true;  
      #endif
    }
    for(int j = 0; j < rSize; ++j) {
      delete split[j];
    }
    delete[] split; 
  } else {
    if (arg == "f") {
      pr_wc     = true;               
      pr_wcF    = true;               
      #ifdef ALSI_ENABLED
      pr_alsi   = true;
      pr_alsiF  = true;    
      #endif               
    }
    else if (arg == "c") {
      pr_wc     = true;   
      #ifdef ALSI_ENABLED            
      pr_alsi   = true;   
      #endif            
    }
    else {
      if (arg == "wc")     pr_wc     = true;               
      if (arg == "wci")    pr_wcF    = true;  
      #ifdef ALSI_ENABLED             
      if (arg == "alsi")   pr_alsi   = true;               
      if (arg == "alsii")  pr_alsiF  = true;
      #endif
    }

  }


  JsonArray wccat, wccat_item, wccat_subitem_a1, wccat_subitem_a2;
  JsonObject  main        = doc.createNestedObject(F("api"));

  JsonArray key = main.createNestedArray(F("api-keys"));
  key.add(F("wc"));
  key.add(F("wci"));
  key.add(F("alsi"));
  key.add(F("alsii"));
  
  if (pr_wc) {
    JsonObject  wc          = main.createNestedObject(F("wc"));
                wccat       = wc.createNestedArray(F("cat"));
                wccat_item  = wccat.createNestedArray();
    wccat_item.add(0);
    wccat_item.add(FPSTR(WCEVO_PTJSON_001));
    wccat_item = wccat.createNestedArray();
    wccat_item.add(1);
    wccat_item.add(FPSTR(WCEVO_PTJSON_002));
    wccat_item = wccat.createNestedArray();
    wccat_item.add(2);
    wccat_item.add(FPSTR(WCEVO_PTJSON_003));
    if (pr_wcF) {
      wccat_subitem_a1 = wccat_item.createNestedArray();
      wccat_subitem_a2 = wccat_subitem_a1.createNestedArray();
      wccat_subitem_a2.add(3);
      wccat_subitem_a2.add(FPSTR(WCEVO_PTJSON_004)); 
      wccat_subitem_a2 = wccat_subitem_a1.createNestedArray();
      wccat_subitem_a2.add(4);
      wccat_subitem_a2.add(FPSTR(WCEVO_PTJSON_005)); 
      wccat_subitem_a2 = wccat_subitem_a1.createNestedArray();
      wccat_subitem_a2.add(5);
      wccat_subitem_a2.add(FPSTR(WCEVO_PTJSON_006)); 
    }
    wccat_item = wccat.createNestedArray();
    wccat_item.add(6);
    wccat_item.add(FPSTR(WCEVO_PTJSON_007));
    if (pr_wcF) {
      wccat_subitem_a1 = wccat_item.createNestedArray();
      wccat_subitem_a2 = wccat_subitem_a1.createNestedArray();
      wccat_subitem_a2.add(7);
      wccat_subitem_a2.add(FPSTR(WCEVO_PTJSON_008)); 
      wccat_subitem_a2 = wccat_subitem_a1.createNestedArray();
      wccat_subitem_a2.add(8);
      wccat_subitem_a2.add(FPSTR(WCEVO_PTJSON_009)); 
      wccat_subitem_a2 = wccat_subitem_a1.createNestedArray();
      wccat_subitem_a2.add(9);
      wccat_subitem_a2.add(FPSTR(WCEVO_PTJSON_010)); 
    }
    wccat_item = wccat.createNestedArray();
    wccat_item.add(10);
    wccat_item.add(FPSTR(WCEVO_PTJSON_011));
  }


  JsonArray arr_1, arr_2, arr_3, arr_4;
  arr_1 = doc[F("api")][F("wc")][F("cat")];
  for(size_t i = 0; i < arr_1.size(); ++i) {
    arr_2 = arr_1[i];
    uint8_t iS = arr_2[0].as<uint8_t>();
    String  iP = arr_2[1].as<String>();
    Serial.printf_P(PSTR("[%d] pos: %d - name: %s\n"), i, iS, iP.c_str());

    if (arr_2[2]) {
      arr_3 = arr_2[2];
      for(size_t j = 0; j < arr_3.size(); ++j) {
        arr_4 = arr_3[j];
        uint8_t iS = arr_4[0].as<uint8_t>();
        String  iP = arr_4[1].as<String>();
        Serial.printf_P(PSTR("\t[%d] pos: %d - name: %s\n"), j, iS, iP.c_str());          
      }
    }

  }

  

  #ifdef ALSI_ENABLED

    if (pr_alsi) {
      JsonObject  alsi = main.createNestedObject(F("alsi"));
      wccat = alsi.createNestedArray(F("cat"));
      for(int i = 0; i < ALSI_CATEGORYSIZE; ++i) {
        wccat_item = wccat.createNestedArray();
        wccat_item.add(i);
        wccat_item.add(FPSTR(ALSI_CATEGORY[i])); 
        if (pr_alsiF){
          wccat_subitem_a1 = wccat_item.createNestedArray();
          for(int j = 0; j < ALSI_ITEMSSIZE; ++j) {
            if (ALSI_items[j].GRP == ALSI_CATEGORY[i]) {
              wccat_subitem_a2 = wccat_subitem_a1.createNestedArray();
              wccat_subitem_a2.add(j);
              wccat_subitem_a2.add(FPSTR(ALSI_items[j].NAME));           
            }
          }   
        }   
      }   
    }

    Serial.println("--");
    arr_1 = doc[F("api")][F("alsi")][F("cat")];
    for(size_t i = 0; i < arr_1.size(); ++i) {
      arr_2 = arr_1[i];
      uint8_t iS = arr_2[0].as<uint8_t>();
      String  iP = arr_2[1].as<String>();
      Serial.printf_P(PSTR("[%d] pos: %d - name: %s\n"), i, iS, iP.c_str());

      if (arr_2[2]) {
        arr_3 = arr_2[2];
        for(size_t j = 0; j < arr_3.size(); ++j) {
          arr_4 = arr_3[j];
          uint8_t iS = arr_4[0].as<uint8_t>();
          String  iP = arr_4[1].as<String>();
          Serial.printf_P(PSTR("\t[%d] pos: %d - name: %s\n"), j, iS, iP.c_str());          
        }
      }

    }

  #endif

}
void WCEVO_manager::keyboard_print() {
  Serial.printf_P(PSTR("@&wc:0,server=\n"));
  Serial.printf_P(PSTR("[%-3d] %s\n"), 0, WCEVO_PTJSON_001);
  Serial.printf_P(PSTR("[%-3d] %s\n"), 1, WCEVO_PTJSON_002);
  Serial.printf_P(PSTR("[%-3d] %s\n"), 2, WCEVO_PTJSON_003);
  Serial.printf_P(PSTR("[%-3d]\t%s\n"), 3, WCEVO_PTJSON_004);
  Serial.printf_P(PSTR("[%-3d]\t%s\n"), 4, WCEVO_PTJSON_005);
  Serial.printf_P(PSTR("[%-3d]\t%s\n"), 5, WCEVO_PTJSON_006);
  Serial.printf_P(PSTR("[%-3d] %s\n"), 6, WCEVO_PTJSON_007);
  Serial.printf_P(PSTR("[%-3d]\t%s\n"), 7, WCEVO_PTJSON_008);
  Serial.printf_P(PSTR("[%-3d]\t%s\n"), 8, WCEVO_PTJSON_009);
  Serial.printf_P(PSTR("[%-3d]\t%s\n"), 9, WCEVO_PTJSON_010);
  Serial.printf_P(PSTR("[%-3d] %s\n"), 10, WCEVO_PTJSON_011);

}
#ifdef FILESYSTEM
/*
  API
    GET
      credentials
      credential
        _sta_ssid
        _sta_ssidPsk        
      _server
        _ota_psk
        _ap_ssid
        _ap_psk
        _hostname        
      WCEVO_manager
        _credentialUse
        _CONNECTMOD
        _CONNECTFAIL  
      scanNetwork
        loop
        nm  
        
    SET  
      credentials 
        add
        list clear
      credential 
        _sta_ssid
        _sta_ssidPsk             
      set_cm
      set_cmFail 
        m   

    JSON
      credentials   array [["",""],["",""],...]
      credential    array [["",""]]
      cm            object
      cmf           object

  sauvegardes de tous les parametres wifi en une fois
    a modifer pour ne sauvegarder que les parametres voullu
      - plusieur fichies json || system d'ecriture via numero de ligne dans un fichier
  
  items:  [
            ["ssid", "psk"],["ssid", "psk"],...
          ]

  current : [["ssid", "psk"]]
    array = array[0]
    ssid  = array[0]
    psk   = array[1]
*/
void WCEVO_manager::credentials_to_fs(wcevo_connectmod_t cm, wcevo_connectfail_t cmf){
  credentials_to_fs(_credentialUse, cm, cmf);
}
void WCEVO_manager::credentials_to_fs(boolean cu, wcevo_connectmod_t cm, wcevo_connectfail_t cmf){
  File f=FILESYSTEM.open(config_filepath,"w");
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "-\n");
  if (!f) {
    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "[Error open /w]\n\t%s\n", config_filepath);  
    return;
  } 
  DynamicJsonDocument doc(2048);
  get_credentials_json(_temp_WCEVO_credential, doc);
  doc[FPSTR(WCEVO_PTJSON_008)]  = (cm==WCEVO_CM_NONE  )?_CONNECTMOD:cm;
  doc[FPSTR(WCEVO_PTJSON_009)]  = (cmf==WCEVO_CF_NONE )?_CONNECTFAIL:cmf;
  doc[FPSTR(WCEVO_PTJSON_010)]  = cu;
  doc[FPSTR(WCEVO_PTJSON_012)]  = _mode_start;
  serializeJson(doc, f);   
  serializeJsonPretty(doc, Serial);
  Serial.println();   
}
void WCEVO_manager::credentials_from_fs(){
  // ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "read filepath: %s\n", config_filepath);
  File f=FILESYSTEM.open(config_filepath,"r");
  if (!f) {
    // ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "[Error open /r]\n\t%s\n", config_filepath);  
    return;
  } 
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, f);
  JsonArray arr_1;
  JsonArray arr_2;
  if (doc[FPSTR(WCEVO_PTJSON_001)]) {
    arr_1 = doc[FPSTR(WCEVO_PTJSON_001)];
    for(size_t i = 0; i < arr_1.size(); ++i) {
      arr_2 = arr_1[i];
      String iS = arr_2[0].as<String>();
      String iP = arr_2[1].as<String>();
      credentials_add(iS.c_str(), iP.c_str());
    }
  }  
  if (doc[FPSTR(WCEVO_PTJSON_002)]) {
    arr_1 = doc[FPSTR(WCEVO_PTJSON_002)];
    arr_1 = arr_1[0];
    // ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "&c:1&s:->load creadential\n\tssid: %s\n\tpsk: %s\n", arr_1[0].as<String>().c_str(), arr_1[1].as<String>().c_str());
    set_credential(arr_1);
  }
  if (doc.containsKey(FPSTR(WCEVO_PTJSON_012))) {
    uint8_t p = doc[FPSTR(WCEVO_PTJSON_012)].as<uint8_t>();
    // ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "&c:1&s:->load mode start\n\t%s\n", p);
    _mode_start = p;
  }   
  if (doc.containsKey(FPSTR(WCEVO_PTJSON_008))) {
    uint8_t p = doc[FPSTR(WCEVO_PTJSON_008)].as<uint8_t>();
    // ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "&c:1&s:->load cm\n\t%s\n", wcevo_connectmod_s[p]);
    _CONNECTMOD = wcevo_connectmodArray_t[p];
  }  
  if (doc.containsKey(FPSTR(WCEVO_PTJSON_009))) {
    uint8_t p = doc[FPSTR(WCEVO_PTJSON_009)].as<uint8_t>();
    // ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "&c:1&s:->load cmf\n\t%s\n", wcevo_connectfail_s[p]);
    _CONNECTFAIL = wcevo_connectfaildArray_t[p];
  }  
  if (doc.containsKey(FPSTR(WCEVO_PTJSON_010))) {
    bool cu = doc[FPSTR(WCEVO_PTJSON_010)];
    // ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "&c:1&s:->load cu\n\t%d\n", cu);
    _credentialUse = cu;
  }  

} 

#endif

LList<WCEVO_credential *> * WCEVO_manager::credentials() {
  return &_credentials;
}

void WCEVO_manager::credentials_print(){
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "size: %d\n", _credentials.size());
  for(int i = 0; i < _credentials.size(); ++i) {
    WCEVO_credential * item = _credentials.get(i);
    const char * ssid;
    const char * psk;
    item->get_ssid(ssid);
    item->get_psk(psk);
    Serial.printf_P(PSTR("[%d] ssid: %-20s psk: %-20s\n"), i, ssid, psk);
  }
}

void WCEVO_manager::set_cm(wcevo_connectmod_t v1)         {_CONNECTMOD=v1;};
void WCEVO_manager::set_cmFail(wcevo_connectfail_t v1)    {_CONNECTFAIL=v1;_CONNECTFAIL_OLD=v1;};

void WCEVO_manager::set_credentialUse(boolean v1)         {_credentialUse=v1;};
boolean WCEVO_manager::get_credentialUse()            {return _credentialUse;};

void WCEVO_manager::set_scanNetwork_running(boolean v1)   {
  _scanNetwork_running=v1;
  if (!_scanNetwork_running) networkScan()->scan_stop();
};
void WCEVO_manager::set_scanNetwork_requiered(boolean v1) {
  _scanNetwork_requiered=v1;
  if (!_scanNetwork_requiered) networkScan()->scan_stop();
};

boolean WCEVO_manager::get_scanNetwork_running()    {return _scanNetwork_running;};
boolean WCEVO_manager::get_scanNetwork_requiered()  {return _scanNetwork_requiered;};

wcevo_connectmod_t  WCEVO_manager::get_cm()       {return _CONNECTMOD;};
wcevo_connectfail_t WCEVO_manager::get_cmFail()   {return _CONNECTFAIL;};

WiFiManagerCPY      * WCEVO_manager::get_WM()     {return &_WM;}
WCEVO_STAconnect    * WCEVO_manager::get_STA()    {return &_STACO;}
WCEVO_APconnect     * WCEVO_manager::get_AP()     {return &_APCO;}

void WCEVO_manager::mdns_setup(){
  MDNS.end(); 
  #if defined(ESP8266)
    MDNS.begin(_server->get_hostName());  
  #elif defined(ESP32)
    MDNS.begin(_server->get_hostName().c_str());  
  #endif
  MDNS.addService("http", "tcp", 80);
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "mDNS started\n");
}

boolean WCEVO_manager::sta_connected(){
  return _STACO.get_active();
}
boolean WCEVO_manager::isConnected(){
  return (WiFi.localIP()[0] != 0 && WiFi.status() == WL_CONNECTED);
}
IPAddress WCEVO_manager::localIP() {
  IPAddress localIP;
  localIP = WiFi.localIP();
  if (localIP[0] != 0) {
    return localIP;
  }

  return INADDR_NONE;
}


uint8_t WCEVO_manager::sta_getMaxAettemp(){ 
  switch (_CONNECTFAIL) {
    case WCEVO_CF_NEXTAP:
       return 1;
    break;
    case WCEVO_CF_AP:
      return 1;
    break;
    default: return 1; break;
   } 
}
void WCEVO_manager::sta_reconnect(){ 
  _STACO.set_reconnectAttempt(true);

  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "ReconnectAttempt: %d\n", _STACO.get_reconnectAttempt() );

  if (!_credential && _CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_NEXTAP) {
    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "&c:1&s:\t networkscan\n" );
    _STACO.set_lastReconnectAttempt();
    _credential           = nullptr;
    _scanNetwork_running  = true;  
    networkScan()->scan_reset();    
  } else { 
    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "&c:1&s:\t initSTA\n" );
    _STACO.setup();
  }  
}
uint8_t WCEVO_manager::sta_reconnect_end(boolean apSetup){
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "-\n");
  // if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_RESET || _CONNECTFAIL_OLD == wcevo_connectfail_t::WCEVO_CF_RESET) ESP.restart();          
  uint8_t attemp = _STACO.set_reconnectAttempt(false);
  if (_CONNECTFAIL != wcevo_connectfail_t::WCEVO_CF_NEXTAP) _STACO.set_lastReconnectAttempt();
  _credential           = nullptr;
  _scanNetwork_running  = false;
  networkScan()->scan_reset();
  if (apSetup && !_APCO.get_active()) _APCO.setup();
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "--\n");
  return attemp;
}

uint32_t lastReconnectAttempt = 0;
void WCEVO_manager::sta_loop(){

  /*
    network scanner enabled by WCEVO_CM_STA || WCEVO_CM_STAAP if reconnecting attemp is needed

    le debut du loop commence via le scan wifi si multiple credential

  */

  if (_scanNetwork_running) {
    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "SCAN network\n");
    if (networkScan()->scan(3)){
      ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "NETWORKSCAN DONE -> Begin sta connection\n");
      networkScan()->list_sortByBestRSSI();
      networkScan()->list_print();  
      _credential = credential_getBestSSID();
      _scanNetwork_running = false;
      if (!_STACO.setup()) {
        if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_RESET) ESP.restart();          
        else if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_CALLBACK) { if ( _cb_cfFail ) _cb_cfFail(); }          
        else if (_CONNECTFAIL_OLD == wcevo_connectfail_t::WCEVO_CF_RESET) { ESP.restart(); }          
        sta_reconnect_end();
      }
    }
  } else {
    _STACO.get_lastReconnectAttempt(lastReconnectAttempt);
    if (lastReconnectAttempt == 0) {
      ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "NETWORKSCAN FALSE -> Begin sta connection\n");
      _STACO.set_reconnectAttempt(true);
      if (!_STACO.setup()) {
         sta_reconnect_end();
      }
    }
  }

  /*
    A FAIRE
    si deco alor reset du mod ap et sta
  */
  if (_STACO.get_wasConnected() && !isConnected() && _STACO.get_serverInitialized() ) { 
    Serial.println("DISCONNECTED !!!");
    // ESP.restart();
  }

  if ( !isConnected()  ) {

    if (_cb_serverEvent_loaded) _cb_serverEvent_loaded = false;

    if (_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STA || _CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STAAP ) {

      _STACO.get_lastReconnectAttempt(lastReconnectAttempt);
      if (_STACO.get_reconnectAttempt() <= sta_getMaxAettemp() && !_APCO.get_active() && !_scanNetwork_running && (millis() - lastReconnectAttempt > 45000) ){
        sta_reconnect();
      } 

      _STACO.get_lastReconnectAttempt(lastReconnectAttempt);
      if (!_APCO.get_active() && !_scanNetwork_running && (millis() - lastReconnectAttempt > 35000) ){
        if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_NEXTAP) {
          if (_STACO.get_reconnectAttempt() > sta_getMaxAettemp()-1 ) {
            if (_scanNetwork_gotSSID) {
              ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "WCEVO_CF_NEXTAP -> reconnect ? end\n")
              sta_reconnect_end();
            } else {
              ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "WCEVO_CF_NEXTAP -> reconnect ? next\n")
              sta_reconnect_end(false);
            }
          }   
        } else if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_AP) {
          if (_STACO.get_reconnectAttempt() > sta_getMaxAettemp()-1 ) {
            ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "WCEVO_CF_AP -> reconnect\n")
            sta_reconnect_end();
          } 
        } else if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_RESET) {
          if (_STACO.get_reconnectAttempt() > sta_getMaxAettemp()-1 ) {
            ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "WCEVO_CF_RESET -> EST.restart\n");
            ESP.restart();
          } 
        } else if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_CALLBACK) {
          if (_STACO.get_reconnectAttempt() > sta_getMaxAettemp()-1 ) {
            ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "WCEVO_CF_CALLBACK -> WCEVO_CF_CALLBACK\n");
            if ( _cb_cfFail ) _cb_cfFail();
          } 
        }  

      }    
    }
  } else if (!_STACO.get_serverInitialized()){

    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "-\n");

    if      (WCEVO_managerPtrGet()->get_cm()==WCEVO_CM_STAAP)  WiFi.mode(WIFI_AP_STA);
    else if (WCEVO_managerPtrGet()->get_cm()==WCEVO_CM_STA)    WiFi.mode(WIFI_STA);
    else WiFi.mode(WIFI_STA);

    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "---\n");

    yield();

    uint8_t attemp = sta_reconnect_end(false);

    _STACO.get_lastReconnectAttempt(lastReconnectAttempt);
    uint32_t diff = millis() - lastReconnectAttempt ;
    String diffStr; al_tools::on_time_h(diff, diffStr);
    Serial.printf_P(PSTR("\n\t>>> Connected to IP address:"));
    Serial.print(localIP());
    Serial.printf_P(PSTR(" in: %s attemp: %d - from: %s\n"), diffStr.c_str(), attemp, WiFi.SSID().c_str());

    if (_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STAAP){  
      if (!_APCO.get_active()) _APCO.setup(false, false, false);
    } 

    if (!_APCO.get_active()) mdns_setup();

#ifdef ALWC_WS_OTA
     ota_setup();  
#endif 

    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "Init STA interfaces\n");
    
    if (_cb_webserverEvent!=nullptr) {
      ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "STA setup external event webserver handle\n");
      _cb_webserverEvent();}
    
    if (_cb_webserverOn!=nullptr) {
       ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "STA setup external http webserver handle\n");
       _cb_webserverOn();
     }
     
#ifdef ALWC_WS_API
         ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "STA register HTTP_GET request : /wcapi\n"); 
         _webserver->on("/wcapi", HTTP_GET, [this](AsyncWebServerRequest *request){
           DynamicJsonDocument doc(5000);
           int rSize = 0;
           for (unsigned int i = 0; i < request->args(); i++) {
             // message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
             Serial.printf_P(PSTR("argName: %s arg: %s\n"), request->argName(i).c_str(), request->arg(i).c_str());
     
             if (request->argName(i) == "api") {
               api_key(doc, request->arg(i) );
             } 
     
             if (request->argName(i) == "set_cm") {
               // 
               wcevo_connectmod_t mod = wcevo_connectmodArray_t[request->arg(i).toInt()];
               set_cm(mod);
               api_getter(doc, "server");
               api_getter(doc, "connection");
             } 
     
             if (request->argName(i) == "set_cmFail") {
               // 
               wcevo_connectfail_t mod = wcevo_connectfaildArray_t[request->arg(i).toInt()];
               set_cmFail(mod);
               api_getter(doc, "server");
               api_getter(doc, "connection"); 
             } 
     
             if (request->argName(i) == "to_fs") {
               #ifdef FILESYSTEM
                 credentials_to_fs();
                 credentials_from_fs();
                 api_getter(doc, "server");
                 api_getter(doc, "connection");   
               #endif          
             } 
     
             rSize = 0;
             const char** split = al_tools::explode(request->arg(i), ',', rSize);
             if (split) {
               for(int j = 0; j < rSize; ++j) {
                 Serial.printf_P(PSTR("[%d] %s\n"), j , split[j]);
                 if (request->argName(i) == "wc")      api_getter(doc, split[j]);                           
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
               if (request->argName(i) == "wc")      api_getter(doc, request->arg(i).c_str());                           
               #ifdef ALSI_ENABLED
               if (request->argName(i) == "alsi")    ALSYSINFO_getterByCat(doc, request->arg(i).c_str());                           
               if (request->argName(i) == "alsii")   ALSYSINFO_getterByKey(doc, request->arg(i).c_str());   
               #endif           
             }       
           }    
           String result; 
           serializeJson(doc,result); 
           request->send(200, "application/json", result);
         }).setFilter(ON_STA_FILTER);  
#endif     

    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "webserverBegin\n");
    _webserver->begin();
    // 
    
    _STACO.set_serverInitialized(true); 
    _STACO.set_wasConnected(true);

    delay(0);

    ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "--\n"); 
    
  }
}
/*
void WCEVO_manager::keyboard_getter(const String & v1) {
  int rSize = 0;
  DynamicJsonDocument doc(3500);

  LList<SplitItem *> _SplitItem;
  splitText(v1, "&",  ':', &_SplitItem);

  for(int j = 0; j < _SplitItem.size(); ++j) {
    const char** split = al_tools::explode(_SplitItem[j]->_value, ',', rSize);
    if (split) {
      for(int i = 0; i < rSize; ++i) {
        Serial.printf("[%d] %s\n", i , split[i]);
        if (strcmp_P(_SplitItem[j]->_cmd, "WC") == 0)     api_getter(doc, split[i]);                           
        #ifdef ALSI_ENABLED
        if (strcmp_P(_SplitItem[j]->_cmd, "ALSI") == 0)   ALSYSINFO_getterByCat(doc, split[i]);                           
        if (strcmp_P(_SplitItem[j]->_cmd, "ALSII") == 0)  ALSYSINFO_getterByKey(doc, split[i]);   
        #endif                          
      }
      for(int i = 0; i < rSize; ++i) {
        delete split[i];
      }
      delete[] split; 
    }
  }
  while (_SplitItem.size()) {
    SplitItem *eff = _SplitItem.shift();
    delete eff;
  }
  _SplitItem.clear();

  serializeJsonPretty(doc,Serial);Serial.println(); 
   
}
*/
void WCEVO_manager::handleConnection(){
  if (_scanNetwork_requiered) {
    Serial.printf_P(PSTR("-tScan\n"));
    if (networkScan()->scan(1)){
      networkScan()->list_sortByBestRSSI();
      networkScan()->scan_reset();  
      networkScan()->list_print();  
      _scanNetwork_requiered = false;
    }  
  }

  if (_STACO.get_active() || _APCO.get_active()){
    #if defined(ESP8266)
      MDNS.update();
    #endif  
    #ifdef ALWC_WS_OTA
      ArduinoOTA.handle();  
    #endif
    if (!_cb_serverEvent_loaded) {
      if (_cb_serverEvent) _cb_serverEvent();
      _cb_serverEvent_loaded = true;
    }
  }

  if (_updateMod != wcevo_updateMod_t::WCEVO_UM_UPDATED && _APCO.get_active()){
    if (WCEVO_CONNECTED && _CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STAAP){
      _updateMod = wcevo_updateMod_t::WCEVO_UM_STAAP;
    }  
    else if (WCEVO_CONNECTED &&_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STA){
      _updateMod = wcevo_updateMod_t::WCEVO_UM_STA;
    }
    else if (!_STACO.get_active()){
      _updateMod = wcevo_updateMod_t::WCEVO_UM_AP;
    }    
  }


  if ((_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STA ||_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STAAP) && !_APCO.get_active()){
    sta_loop();
  }
  if (_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_AP && !_APCO.get_active() ) {
    _APCO.setup();
  }
  if (_APCO.get_active() ) {
    // configPortalHasTimeout();
    if (_configPortalAuto == 0) {
      _dnsServer->processNextRequest(); 
    } else if (_configPortalAuto == 1) {
      configPortalHasTimeout();
      if (_configPortalMod == 1) _dnsServer->processNextRequest(); 
    } 
    //   
  }
  
}
boolean WCEVO_manager::configPortalHasTimeout(){
  // if(_configPortalTimeout == 0 || wifi_softap_get_station_num() > 0){
  //   _configPortalStart = millis(); // kludge, bump configportal start time to skew timeouts
  //   return false;
  // }
  // return (millis() > _configPortalStart + _configPortalTimeout);


  #ifdef ESP8266
    if (_configPortalMod == 2) {
      return false;
    } else if ( (_configPortalMod == 0) && (wifi_softap_get_station_num() > 0) ) {
      _configPortalMod = 1;
      _configPortalStart = millis();
      Serial.println(F("Starting portal\n"));
    } else if ( (_configPortalMod == 1) && ( (millis()-_configPortalStart) > _configPortalTimeout) ) {
      _configPortalMod = 2;
      _configPortalAuto =2;
      Serial.println(F("Stopped portal\n"));
      return true;
    }  
  #endif
  return false;
}
void WCEVO_manager::setConfigPortalTimeout(unsigned long seconds) {
  _configPortalTimeout = seconds * 1000;
  _configPortalMod = 0;
  _configPortalAuto = 1;
}

void WCEVO_manager::ota_setup(){
  if (_otaSetup) return;

  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "-\n");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf_P(PSTR("Progress: %u%%\n"), (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.setHostname(_server->get_hostName().c_str());
  ArduinoOTA.begin();

  _otaSetup = true;
  ALT_TRACEC(WCEVO_DEBUGREGION_WCEVO, "&c:2");
}






/*
#if defined(ESP32)
void display_connected_devices()
{
  static byte stacO = 0;

    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

    if (adapter_sta_list.num == stacO) return;
    stacO = adapter_sta_list.num;

    if (adapter_sta_list.num > 0)
        Serial.println("-----------");
    for (uint8_t i = 0; i < adapter_sta_list.num; i++)
    {
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        Serial.print((String)"[+] Device " + i + " | MAC : ");
        Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5]);
        Serial.println((String) " | IP " + ip4addr_ntoa(&(station.ip)));
    }
}
#elif defined(ESP8266)
void display_connected_devices()
{
  // https://stackoverflow.com/questions/42593385/get-mac-address-of-client-connected-with-esp8266
  static byte stacO = 0;

  unsigned char number_client = wifi_softap_get_station_num();
  
  if (number_client == stacO) return;
  stacO = number_client;

  
  
  
  Serial.print(" Total Connected Clients are = ");
  Serial.println(number_client);

    auto i = 1;
    struct station_info *station_list = wifi_softap_get_station_info();
    while (station_list != NULL) {
        auto station_ip = IPAddress((&station_list->ip)->addr).toString().c_str();
        char station_mac[18] = {0};
        sprintf(station_mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station_list->bssid));
        Serial.printf("%d. %s %s\n", i++, station_ip, station_mac);
        station_list = STAILQ_NEXT(station_list, next);
    }
    wifi_softap_free_station_info();
}
#endif
*/