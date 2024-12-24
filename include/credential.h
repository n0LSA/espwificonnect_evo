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

#ifndef _CREDENTIAL_H
#define _CREDENTIAL_H
  #include "def.h"

  class WCEVO_server
  {
      char * _ota_psk   = nullptr;
      char * _ap_ssid   = nullptr;
      char * _ap_psk    = nullptr;
      char * _hostname  = nullptr;
      char * ApSSIDInit(const char * const & str);
  public:
      ~WCEVO_server();
      WCEVO_server( const char * const &, const char * const & , const char * const & );  
      WCEVO_server( WCEVO_server* ptr );  
      WCEVO_server();
      String get_hostName();
      void get_hostName(const char *& result);
      void get_apSSID(const char *& result);
      void get_apPsk(const char *& result); 
      void get_otaPsk(const char *& result);  
      void print();
  };
  class WCEVO_credential
  {
    char    * _sta_ssid     = nullptr;
    char    * _sta_ssidPsk  = nullptr;
    uint8_t _order          = 0;
    boolean _tested         = false;
  public:
    ~WCEVO_credential();
    WCEVO_credential( const char * const &, const char * const &  );  
    void get_ssid(const char *& result);
    String get_ssid();
    String get_psk();
    boolean get_tested();
    void get_psk(const char *& result); 
    void set_tested(boolean);
    void print();

    void set_psk(const char * const &, const char * const & );
  };


#endif // CREDENTIAL_H