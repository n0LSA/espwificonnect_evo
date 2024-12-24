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

#include "../include/credential.h"

size_t bufferInit(char * & buffer, const char * const & str){
  if (!buffer) {
    size_t len = strlen(str);
    buffer = new char[len+1];
    strcpy(buffer, str);
  } 
  return strlen(buffer);
}
char * WCEVO_server::ApSSIDInit(const char * const & str){
  String apHost = String(str);
  apHost.replace("_", "");
  apHost.toLowerCase();  
  byte  apHostLen = apHost.length();
  char  ch[apHostLen+1]; 
  byte  len = (apHostLen>8)?8:apHostLen;
  char * buffer = new char[len+8]; // ("host" + mac[6] + "-" )+1
  strcpy(buffer,"");
  strcpy(ch, apHost.c_str()); 
  for(int i=0; i < len; i++) {strcat(buffer, String(ch[i]).c_str());}
  strcat(buffer, "-");
  unsigned char mac[6];
  WiFi.macAddress(mac);
  for (int i=3; i<6; i++) {
    char b[3]; 
    sprintf(b,"%02x",mac[i]); 
    strcat(buffer, b); 
  } 
  return buffer;
}


WCEVO_server::~WCEVO_server(){
  if (_ota_psk)   delete _ota_psk;
  if (_ap_ssid)   delete _ap_ssid;
  if (_ap_psk)    delete _ap_psk;
  if (_hostname)  delete _hostname;
}

WCEVO_server::WCEVO_server(){
  bufferInit(_ota_psk,  WCEVO_OTAPSK);
  bufferInit(_hostname, WCEVO_HOSTNAME);
  bufferInit(_ap_psk,   WCEVO_APPSK);
  if (!_ap_ssid) {
    char * buffer = ApSSIDInit(_hostname);
    if (buffer) {
      bufferInit(_ap_ssid, buffer);
      delete buffer;      
    }
  }
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "OTAPSK",     WCEVO_OTAPSK,     "_ota_psk",     _ota_psk);
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "HOSTNAME",   WCEVO_HOSTNAME,   "_hostname",    _hostname);
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "",           "",               "_ap_ssid",     _ap_ssid);  
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "APPSK",    WCEVO_APPSK,      "_ap_psk",      _ap_psk);
}
WCEVO_server::WCEVO_server(
  const char * const & Host, 
  const char * const & APpass, 
  const char * const & OTApass  ) {
  bufferInit(_ota_psk, OTApass);
  bufferInit(_hostname, Host);
  bufferInit(_ap_psk, APpass);
  if (!_ap_ssid) {
    char * buffer = ApSSIDInit(_hostname);
    if (buffer) {
      bufferInit(_ap_ssid, buffer);
      delete buffer;      
    }
  }
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "OTAPSK",     OTApass, "_ota_psk",        _ota_psk);
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "HOSTNAME",   Host,     "_hostname",      _hostname);
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "",           "",       "_ap_ssid",       _ap_ssid);  
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "APPSK",    APpass,   "_ap_psk",        _ap_psk);
}
WCEVO_server::WCEVO_server(WCEVO_server*ptr){
  const char * apPsk;
  const char * hostname;
  const char * otapsk;
  ptr->get_otaPsk(otapsk);
  ptr->get_hostName(hostname);
  ptr->get_apPsk(apPsk);
  bufferInit(_ota_psk, otapsk);
  bufferInit(_hostname, hostname);
  bufferInit(_ap_psk,   apPsk);
  if (!_ap_ssid) {
    char * buffer = ApSSIDInit(_hostname);
    if (buffer) {
      bufferInit(_ap_ssid, buffer);
      delete buffer;      
    }
  }
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "OTAPSK",     otapsk,     "_ota_psk",     _ota_psk);  
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "HOSTNAME",   hostname,   "_hostname",    _hostname);
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "",           "",         "_ap_ssid",     _ap_ssid);  
  // Serial.printf_P(PSTR("%-10s: %-20s | %-15s: %-20s\n"), "APPSK",    apPsk,      "_ap_psk",      _ap_psk);
}
String WCEVO_server::get_hostName() {
  String result; 
  (_hostname)?  result = al_tools::ch_toString(_hostname): result = ""; 
  return result;
}
void WCEVO_server::get_hostName(const char *& result) {(_hostname)? result = _hostname: result="";}
void WCEVO_server::get_apSSID(const char *& result)   {(_ap_ssid)?  result = _ap_ssid: result="";}
void WCEVO_server::get_apPsk(const char *& result)    {(_ap_psk)?   result = _ap_psk: result="";}
void WCEVO_server::get_otaPsk(const char *& result)   {(_ota_psk)?  result = _ota_psk: result="";}
void WCEVO_server::print(){
  Serial.printf_P(PSTR("%-15s: %-20s\n"), "_ota_psk",     _ota_psk);  
  Serial.printf_P(PSTR("%-15s: %-20s\n"), "_hostname",    _hostname);
  Serial.printf_P(PSTR("%-15s: %-20s\n"), "_ap_ssid",     _ap_ssid);  
  Serial.printf_P(PSTR("%-15s: %-20s\n"), "_ap_psk",      _ap_psk); 
}



WCEVO_credential::WCEVO_credential(const char * const & v1, const char * const & v2){
  bufferInit(_sta_ssid,     v1);
  bufferInit(_sta_ssidPsk,  v2);
}
WCEVO_credential::~WCEVO_credential(){
  if (_sta_ssid)    delete _sta_ssid;
  if (_sta_ssidPsk) delete _sta_ssidPsk;
}
void WCEVO_credential::get_ssid(const char *& result) {(_sta_ssid)?     result = _sta_ssid: result="";}
String WCEVO_credential::get_ssid() {
  String result; 
  (_sta_ssid)? result = al_tools::ch_toString(_sta_ssid): result = "";
  return result;
}
String WCEVO_credential::get_psk() {
  String result; 
  (_sta_ssidPsk)? result = al_tools::ch_toString(_sta_ssidPsk): result = "";
  return result;
}

void WCEVO_credential::get_psk(const char *& result)  {(_sta_ssidPsk)?  result = _sta_ssidPsk: result="";}
boolean WCEVO_credential::get_tested() { return _tested;}
void WCEVO_credential::set_tested(boolean v1) {
  _tested = v1;
}

void WCEVO_credential::print(){
  Serial.printf_P(PSTR("[x] ssid: %-20s psk: %-20s\n"),_sta_ssid, _sta_ssidPsk);
}
void WCEVO_credential::set_psk(const char * const & v1, const char * const &  v2){
  if (_sta_ssid)    {delete _sta_ssid;    _sta_ssid = nullptr;}
  if (_sta_ssidPsk) {delete _sta_ssidPsk;   _sta_ssidPsk = nullptr;}  
  bufferInit(_sta_ssid,     v1);
  bufferInit(_sta_ssidPsk,  v2);  
}
