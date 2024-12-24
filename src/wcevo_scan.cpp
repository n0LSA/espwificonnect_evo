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

#include "../include/wcevo_scan.h"
#include "../include/al_wificonnectevo.h"

#ifdef DEBUG_ESP_WIFI
#ifdef DEBUG_ESP_PORT
#define DEBUG_WIFI_MULTI(fmt, ...) DEBUG_ESP_PORT.printf_P( (PGM_P)PSTR(fmt), ##__VA_ARGS__ )
#endif
#endif

#ifndef DEBUG_WIFI_MULTI
#define DEBUG_WIFI_MULTI(...) do { (void)0; } while (0)
#endif

namespace wcevo_scan {
  String encryptionTypeStr(uint8_t authmode) {
  #if defined(ESP8266)
    switch(authmode) {
      case ENC_TYPE_NONE: return "OPEN";
      case ENC_TYPE_WEP:  return "WEP";
      case ENC_TYPE_TKIP: return "WPA_PSK";
      case ENC_TYPE_CCMP: return "WPA2_PSK";
      case ENC_TYPE_AUTO: return "WPA_WPA2_PSK";
      default:return "UNKOWN";
    }
  #elif defined(ESP32)
    switch(authmode) {
      case 0:return "OPEN";
      case 1:return "WEP";
      case 2:return "WPA_PSK";
      case 3:return "WPA2_PSK";
      case 4:return "WPA_WPA2_PSK";
      case 5:return "WPA2_ENTERPRISE";
      case 6:return "AUTH_MAX";
      default:return "UNKOWN";
    }
  #endif
  }
}



WCEVO_wifiscanItem::WCEVO_wifiscanItem(uint8_t index){
  _index              = index;
  _ssid               = WiFi.SSID(index);
  _encryptionType     = WiFi.encryptionType(index);
  _encryptionTypeStr  = wcevo_scan::encryptionTypeStr(_encryptionType);
  _rssi               = WiFi.RSSI(index);
  _bssidstr           = WiFi.BSSIDstr(index);
  _channel            = WiFi.channel(index);
  _isHidden           = false;   
  #if defined(ESP8266)    
  _isHidden           = WiFi.isHidden(index);       
  #elif defined(ESP32)
  #endif
  uint8_t * mac = WiFi.BSSID(index);
  if (!_mac) _mac = new uint8_t[6]; 
  for ( int j = 0 ; j < 6 ; j++ ) { 
    _mac[j] = mac[j];
  }   

  String mac_1 = "";
  int splitSize;
  const char** list = al_tools::explode(_bssidstr, ':', splitSize);
  for(int i = 1; i < splitSize; ++i) {mac_1+=list[i];}
  // Serial.printf("mac_1: %s\n", mac_1.c_str());
  for(int i = 0; i < splitSize; ++i) {
    delete list[i];
  }
  delete[] list;

}      
WCEVO_wifiscanItem::WCEVO_wifiscanItem(WCEVO_wifiscanItem * item){
  this->_ssid               = item->_ssid;
  this->_encryptionType     = item->_encryptionType;
  this->_encryptionTypeStr  = item->_encryptionTypeStr;
  this->_rssi               = item->_rssi;
  this->_bssidstr           = item->_bssidstr;
  this->_channel            = item->_channel;
  this->_isHidden           = item->_isHidden;
  if (!this->_mac) this->_mac = new uint8_t[6]; 
  for ( int j = 0 ; j < 6 ; j++ ) { 
    this->_mac[j] = item->_mac[j];
  }       
};

void WCEVO_wifiscanItem::update(WCEVO_wifiscanItem * item){
  if (this->_rssi != item->_rssi) {
    DEBUG_WIFI_MULTI("[WIFI] UPD RSSI [%-20s] from %4d to %4d\n", _ssid.c_str(), _rssi, item->_rssi);
    this->_rssi = item->_rssi;}
};  
    

void WCEVO_wifiscanItem::get_chanel(int32_t &result)  {result = _channel;}
void WCEVO_wifiscanItem::get_bssid(String &result)    {result = _bssidstr;}
void WCEVO_wifiscanItem::get_ssid(String &result)     {result = _ssid;}
void WCEVO_wifiscanItem::get_rssi(int32_t &result)    {result = _rssi;}
void WCEVO_wifiscanItem::get_encryptionType(uint8_t &result)  {result = _encryptionType;}
uint8_t * WCEVO_wifiscanItem::get_mac(WCEVO_wifiscanItem*ptr) {
  uint8_t * mac = new uint8_t[6]; 
  for ( int j = 0 ; j < 6 ; j++ ) { 
    mac[j] = ptr->_mac[j];
  } 
  return mac;
}


void WCEVO_wifiscanItem::print(uint8_t p){
  Serial.printf_P(PSTR("%-4d %-4d %-18s %-4d %-20s %-4d %-15s\n"),
    p,
    _rssi,
    _bssidstr.c_str(),
    _isHidden,
    _ssid.c_str(),
    (int)_channel,
    _encryptionTypeStr.c_str()
  );
}

unsigned int WCEVO_scanNetwork::getRSSIasQuality(int RSSI) {
  unsigned int quality = 0;
  if      (RSSI <= -100)  {quality = 0;}
  else if (RSSI >= -50)   {quality = 100;}
  else                    {quality = 2 * (RSSI + 100);}
  return quality;
}
void WCEVO_scanNetwork::networkListAsJson(uint8_t nb, JsonArray & doc) {

  scan(nb);
  list_sortByBestRSSI();
  for (int i = 0; i < _wifiScan.size() ; i++) {
    
    JsonArray   arr = doc.createNestedArray();
    JsonObject  obj = arr.createNestedObject();

    WCEVO_wifiscanItem * sID = _wifiScan.get(i);
    int rssi = 0;
    uint8_t encryptionType = 0;
    int32_t chanel = 0;
    String ssid = "";
    String bssid = "";
    sID->get_rssi(rssi);
    sID->get_ssid(ssid);
    sID->get_encryptionType(encryptionType);
    sID->get_chanel(chanel);
    sID->get_bssid(bssid);

    unsigned int quality = getRSSIasQuality(rssi);
    quality = map(quality, 0, 100, 0, 4);
    Serial.printf_P(PSTR("[%-3d] %-20s rssi: %-3d q1: %-3d\n"), i, ssid.c_str(), rssi, quality);

    obj[F("ssid")] = ssid;
    obj[F("rssi")] = getRSSIasQuality(rssi);
    obj[F("encryptionType")] = wcevo_scan::encryptionTypeStr(encryptionType);
    obj[F("chanel")] = chanel;
    obj[F("bssid")] = bssid;

  }

  if (!WCEVO_managerPtrGet()->get_scanNetwork_requiered()) {
    list_clear();
    WCEVO_managerPtrGet()->set_scanNetwork_requiered(true);
  }  
}
String WCEVO_scanNetwork::networkListAsString(boolean statu) {

  String pager = "";
  for (int i = 0; i < _wifiScan.size() ; i++)
  {
    // if (wifiSSIDs[i].duplicate == true)
    // {
    //   continue; // skip dups
    // }
    WCEVO_wifiscanItem * sID = _wifiScan.get(i);
    int rssi = 0;
    uint8_t encryptionType = 0;
    String ssid = "";
    sID->get_rssi(rssi);
    sID->get_ssid(ssid);
    sID->get_encryptionType(encryptionType);

    unsigned int quality = getRSSIasQuality(rssi);
    quality = map(quality, 0, 100, 0, 4);
    Serial.printf_P(PSTR("[%-3d] %-20s rssi: %-3d q1: %-3d\n"), i, ssid.c_str(), rssi, quality);

    #if (WCEVO_PORTAL == WCEVO_PORTAL_UI)
      String item = FPSTR(HTTP_ITEM);
      item.replace("{v}", ssid);
      item.replace("{r}", String(quality));
      #if defined(ESP8266)
      if (encryptionType != ENC_TYPE_NONE) {
      #else
      if (encryptionType != WIFI_AUTH_OPEN) {
      #endif
        item.replace("{l}", "l");
      } else {
        item.replace("{l}", "");
      }
      pager += item;
    #endif
  }

  list_clear();
  
  if (!WCEVO_managerPtrGet()->get_scanNetwork_requiered() && statu) {
    WCEVO_managerPtrGet()->set_scanNetwork_requiered(true);
  }

  return pager;
}

void WCEVO_scanNetwork::list_clear(){
  if (_wifiScan.size() <= 0) return;
  while (_wifiScan.size()) {
    WCEVO_wifiscanItem *ptr = _wifiScan.shift();
    delete ptr;
  }
  _wifiScan.clear();    
}

void WCEVO_scanNetwork::list_sortByBestRSSI(){
  if (_wifiScan.size() <= 0) return;
  // _wifiScan.sort([](WCEVO_wifiscanItem *&a, WCEVO_wifiscanItem *&b){ return strcmp(a->_ssid.c_str(), b->_ssid.c_str());}); // alphabet ssid
  _wifiScan.sort([](WCEVO_wifiscanItem *&a, WCEVO_wifiscanItem *&b){
    int32_t rssi_a;
    int32_t rssi_b;
    a->get_rssi(rssi_a);
    b->get_rssi(rssi_b); 
    return (rssi_b - rssi_a);}); // ascendant -rssi
}

void WCEVO_scanNetwork::list_print(){
  Serial.printf_P(PSTR("%-4s %-4s %-18s %-4s %-20s %-4s %-15s\n"),"nb","rssi","bssid","hid","ssid","chan","encr");    
  for ( int i = 0 ; i < _wifiScan.size() ; i++ ) {
    WCEVO_wifiscanItem * ptr = _wifiScan.get(i); 
    ptr->print(i);
  }
}
boolean WCEVO_scanNetwork::get_knowSSID(uint8_t pos) {
  for ( int i = 0 ; i < _wifiScan.size() ; i++ ) {

    WCEVO_wifiscanItem * item = _wifiScan.get(i); 
    String ssid = "";
    item->get_ssid(ssid);

    WCEVO_manager * ptr = WCEVO_managerPtrGet();

    const char * cred_psk;
    WCEVO_credential * cred = ptr->credentials()->get(pos);
    cred->get_psk(cred_psk);

    if (cred->get_ssid() == ssid) {
      return true;
    }
  } 
  return false;
} 
int WCEVO_scanNetwork::get_bestSSID() {
  for ( int i = 0 ; i < _wifiScan.size() ; i++ ) {
    WCEVO_wifiscanItem * item = _wifiScan.get(i); 
    String ssid = "";
    item->get_ssid(ssid);

    LList<WCEVO_credential *> * list = WCEVO_managerPtrGet()->credentials();
    for(int j = 0; j < list->size(); ++j) {
      yield();
      const char * cred_psk;
      list->get(j)->get_psk(cred_psk);
      Serial.printf_P(PSTR("[%-3d] %-20s | [%-3d] %-20s : %s\n"), i, ssid.c_str(), j, list->get(j)->get_ssid().c_str(), cred_psk);
      if ( !list->get(j)->get_tested() && (list->get(j)->get_ssid() == ssid)) {
        (list)->get(j)->set_tested(true);
        Serial.printf_P(PSTR("\tRETURN %d - %s : %s\n"), j, list->get(j)->get_ssid().c_str(), cred_psk);
        return j;
      }
    }
  } 
  return -1;
} 
WCEVO_wifiscanItem * WCEVO_scanNetwork::get_bestSSID(String * search, uint8_t size) {
  for ( int i = 0 ; i < _wifiScan.size() ; i++ ) {
    WCEVO_wifiscanItem * item = _wifiScan.get(i); 
    String ssid = "";
    item->get_ssid(ssid);
    // 
    for(int j = 0; j < size; ++j) {
      Serial.printf_P(PSTR("[%-3d] %-20s | [%-3d] %s\n"), i, ssid.c_str(), j, search[j].c_str());
      if (search[j] == ssid) return item;
    }
  } 
  return nullptr;
} 

void WCEVO_scanNetwork::get_SSID(String search, uint8_t & listSize, int8_t & pos) {
  listSize = _wifiScan.size();
  int8_t result = -1;
  for ( int i = 0 ; i < _wifiScan.size() ; i++ ) {
    WCEVO_wifiscanItem * item = _wifiScan.get(i); 
    String ssid = "";
    item->get_ssid(ssid);
    if (ssid == search) { result = i; break;}
  } 
  pos = result;
} 

WCEVO_wifiscanItem * WCEVO_scanNetwork::cmp_ssid(const char * const & search) {
  for ( int i = 0 ; i < _wifiScan.size() ; i++ ) {
    WCEVO_wifiscanItem * item = _wifiScan.get(i); 
    String ssid = "";
    item->get_ssid(ssid);
    if (ssid == al_tools::ch_toString(search)) return item;
  }
  return nullptr;
} 
WCEVO_wifiscanItem * WCEVO_scanNetwork::cmp_mac(WCEVO_wifiscanItem * ptr) {
  for ( int i = 0 ; i < _wifiScan.size() ; i++ ) {
    WCEVO_wifiscanItem * item = _wifiScan.get(i); 
    uint8_t c = 0;
    uint8_t * mac_1 = WCEVO_wifiscanItem::get_mac(item);
    uint8_t * mac_2 = WCEVO_wifiscanItem::get_mac(ptr);
    while ( c < 6 && mac_1[c] == mac_2[c] ) c++;
    if (mac_1) delete mac_1;
    if (mac_2) delete mac_2;
    if ( c == 6 ) return item;
  }
  return nullptr;
} 

void WCEVO_scanNetwork::additem(WCEVO_wifiscanItem * ptr) {
  WCEVO_wifiscanItem * item = cmp_mac(ptr);
  if (item) {
    item->update(ptr);
  } else {
    _wifiScan.add( new WCEVO_wifiscanItem(ptr) );
  }
} 


int WCEVO_scanNetwork::scan(){
  // int8_t scanResult = WiFi.scanComplete();
 //  if(scanResult == WIFI_SCAN_RUNNING) {
  //  return 0;
 //  } 
 //  if(scanResult == WIFI_SCAN_FAILED) {
  //  return 0;
 //  } 

  int scanResult = WiFi.scanNetworks();
  if(scanResult == WIFI_SCAN_RUNNING) {
    return scanResult;
  } else if(scanResult >= 0) {
    DEBUG_WIFI_MULTI("[WIFI] scan done\n");
    delay(0);
    DEBUG_WIFI_MULTI("[WIFI] %d networks found\n", scanResult);
    for(int8_t i = 0; i < scanResult; ++i) {  
      WCEVO_wifiscanItem * item = new WCEVO_wifiscanItem(i);
      additem(item);
      delete item;
      delay(0);
      // if (_scanStop) {
      //  _scanStop = false;
      //  WiFi.scanDelete();
      //  return scanResult;
      // }        
    }
    WiFi.scanDelete();
    delay(0);  
    DEBUG_WIFI_MULTI("[WIFI] scanComplete: %d\n", WiFi.scanComplete());
  } 
  return scanResult;
}

void WCEVO_scanNetwork::scan_stop(){_scanStop=true;}
void WCEVO_scanNetwork::scan_reset(){
  _scanStop = false;
  _scanComplete = false;
  _scanCnt = 0;
}

boolean WCEVO_scanNetwork::scan(uint8_t nb){ 
  if (_scanComplete) return false;

  if (_scanCnt >= nb) { 
    _scanComplete = true;
    _scanStop = false;
    return true;
  } 

  if (scan() > 0) {
    _scanCnt++;
  }

  return false;
}


/*
    void wifiScan_sortByBestRSSI(){
      int indices[_wifiScan.size()+1];
      for (int i = 0; i < _wifiScan.size(); i++) {
        indices[i] = i;
      }     
      for (int i = 0; i < _wifiScan.size(); i++) {
        WCEVO_wifiscanItem * item_1 = _wifiScan.get(i); 
        for (int j = i + 1; j < _wifiScan.size(); j++) {
          WCEVO_wifiscanItem * item_2 = _wifiScan.get(j); 
          if (item_2->_rssi > item_1->_rssi) {
            std::swap(indices[i], indices[j]);  
          }
        }
      }
      LList<WCEVO_wifiscanItem *> _temp;
      for ( int i = 0 ; i < _wifiScan.size() ; i++ ) {
        WCEVO_wifiscanItem * ptr = _wifiScan.get(indices[i]); 
        _temp.add( new WCEVO_wifiscanItem(ptr) );
      }  
      while (_wifiScan.size()) {WCEVO_wifiscanItem *ptr = _wifiScan.shift();delete ptr;}
      _wifiScan.clear();  
      for ( int i = 0 ; i < _temp.size() ; i++ ) {
        WCEVO_wifiscanItem * ptr = _temp.get(indices[i]); 
        _wifiScan.add( new WCEVO_wifiscanItem(ptr) );
      }
      while (_temp.size()) {WCEVO_wifiscanItem *ptr = _temp.shift();delete ptr;}
      _temp.clear();                            
    } 
*/  