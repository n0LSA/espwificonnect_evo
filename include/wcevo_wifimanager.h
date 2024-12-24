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
 * WiFiManager.h
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



#ifndef _WCEVO_WIFIMANAGER_H
  #define _WCEVO_WIFIMANAGER_H
  #include "def.h"

  // region ################################################ WIFIMANAGER CONST - https://github.com/tzapu/WiFiManager
  /**
   * strings_en.h
   * engligh strings for
   * WiFiManager, a library for the ESP8266/Arduino platform
   * for configuration of WiFi credentials using a Captive Portal
   *
   * @author Creator tzapu
   * @author tablatronix
   * @version 0.0.0
   * @license MIT
   */

  const char* const WIFI_MODES[] PROGMEM = { "NULL", "STA", "AP", "STA+AP" };

  #if (WCEVO_PORTAL == WCEVO_PORTAL_UI)

  const char WCEVO_HTTP_HEAD[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
      <title>{v}</title>
  )rawliteral"; 
  const char WCEVO_HTTP_STYLE[] PROGMEM = R"rawliteral(
  <style>
  :root{--primarycolor:#1fa3ec}.c,body{text-align:center;font-family:verdana}.wrap{text-align:left;display:inline-block;min-width:260px;max-width:500px}div,input,select{padding:5px;font-size:1em;margin:5px 0;box-sizing:border-box}div{margin:5px 0}.msg,button,input,select{border-radius:.3rem;width:100%}input[type=checkbox],input[type=radio]{width:auto}button,input[type=button],input[type=submit]{border:0;background-color:var(--primarycolor);color:#fff;line-height:2.4rem;font-size:1.2rem}input[type=file]{border:1px solid var(--primarycolor)}a{color:#000;font-weight:700;text-decoration:none}a:hover{color:var(--primarycolor);text-decoration:underline}.h{display:none}.q{height:16px;margin:0;padding:0 5px;text-align:right;min-width:38px;float:right}.q.q-0:after{background-position-x:0}.q.q-1:after{background-position-x:-16px}.q.q-2:after{background-position-x:-32px}.q.q-3:after{background-position-x:-48px}.q.q-4:after{background-position-x:-64px}.q.l:before{background-position-x:-80px;padding-right:5px}.ql .q{float:left}.q:after,.q:before{content:'';width:16px;height:16px;display:inline-block;background-repeat:no-repeat;background-position:16px 0;background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGAAAAAQCAMAAADeZIrLAAAAJFBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADHJj5lAAAAC3RSTlMAIjN3iJmqu8zd7vF8pzcAAABsSURBVHja7Y1BCsAwCASNSVo3/v+/BUEiXnIoXkoX5jAQMxTHzK9cVSnvDxwD8bFx8PhZ9q8FmghXBhqA1faxk92PsxvRc2CCCFdhQCbRkLoAQ3q/wWUBqG35ZxtVzW4Ed6LngPyBU2CobdIDQ5oPWI5nCUwAAAAASUVORK5CYII=')}@media (-webkit-min-device-pixel-ratio:2),(min-resolution:192dpi){.q:after,.q:before{background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALwAAAAgCAMAAACfM+KhAAAALVBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAOrOgAAAADnRSTlMAESIzRGZ3iJmqu8zd7gKjCLQAAACmSURBVHgB7dDBCoMwEEXRmKlVY3L//3NLhyzqIqSUggy8uxnhCR5Mo8xLt+14aZ7wwgsvvPA/ofv9+44334UXXngvb6XsFhO/VoC2RsSv9J7x8BnYLW+AjT56ud/uePMdb7IP8Bsc/e7h8Cfk912ghsNXWPpDC4hvN+D1560A1QPORyh84VKLjjdvfPFm++i9EWq0348XXnjhhT+4dIbCW+WjZim9AKk4UZMnnCEuAAAAAElFTkSuQmCC');background-size:95px 16px}}dt{font-weight:700}dd{margin:0;padding:0 0 .5em 0}td{vertical-align:top}button.D{background-color:#dc3630}button{transition:0s opacity;transition-delay:3s;transition-duration:0s;cursor:pointer}button:active{opacity:50%!important;cursor:wait;transition-delay:0s}input:disabled{opacity:.5}
  </style>
  )rawliteral";
  const char WCEVO_HTTP_SCRIPT[] PROGMEM = R"rawliteral(
  <script>
    function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}
  </script>
  )rawliteral";  
  const char WCEVO_HTTP_HEAD_END[] PROGMEM = R"rawliteral(
    </head>
    <body>
      <div class='wrap'>
  )rawliteral"; 

  const char WCEVO_HTTP_END[] PROGMEM = R"rawliteral(
      </div>
    </body>
  </html>
  )rawliteral";   

  const char HTTP_PORTAL_OPTIONS  [] PROGMEM = {
  "<form action=\"/w\"  method=\"get\"><button>Credential</button></form><br/>"
  "<form action=\"/wm\" method=\"get\"><button>Credential multiple</button></form><br/>"
  "<form action=\"/m\"  method=\"get\"><button>Server setup</button></form><br/>"
  "<form action=\"/ia\" method=\"get\"><button>Info ip</button></form><br/>"
  "<form action=\"/i\"  method=\"get\"><button>Info</button></form><br/>"
  "<form action=\"/r\"  method=\"post\"><button>Reset</button></form><br/>"
  };

  const char R_root               [] PROGMEM = "/";
  const char R_wifi               [] PROGMEM = "/w";
  const char R_wifiMulti          [] PROGMEM = "/wm";
  const char R_wifiScan           [] PROGMEM = "/wsc";
  const char R_wifiScanMulti      [] PROGMEM = "/wscm";
  const char R_wifisave           [] PROGMEM = "/ws";
  const char R_wifisaveMulti      [] PROGMEM = "/wsm";
  const char R_wifico             [] PROGMEM = "/wc";
  const char R_wifimod            [] PROGMEM = "/m";
  const char R_wifisavmod         [] PROGMEM = "/ms";
  const char R_info               [] PROGMEM = "/i";
  const char R_infoAndroid        [] PROGMEM = "/ia";
  const char R_restart            [] PROGMEM = "/r";

  const char HTTP_FORMMULTI_START [] PROGMEM = "<form method='get' action='wsm'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/><input id='c' name='c' length=32 type='number' placeholder='0'><br/>";
  const char HTTP_FORM_START      [] PROGMEM = "<form method='get' action='ws'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/>";
  const char HTTP_FORM_PARAM      [] PROGMEM = "<br/><input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}>";
  const char HTTP_FORM_END        [] PROGMEM = "<br/><button type='submit'>save</button></form>";
  const char HTTP_SCAN_LINK       [] PROGMEM = "<br/><div class=\"c\"><a href=\"/wsc\">Scan</a></div>";
  const char HTTP_SCAN_LINKMULTI  [] PROGMEM = "<br/><div class=\"c\"><a href=\"/wscm\">Scan</a></div>";
  // const char HTTP_SCANR_LINK    [] PROGMEM = "<div class=\"c\"><a href=\"/scanr\">Reset scan</a></div>";
  const char HTTP_HOME_LINK       [] PROGMEM = "<div class=\"c\"><a href=\"/\">Home</a></div>";
  const char HTTP_SAVED           [] PROGMEM = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
  const char HTTP_BR              [] PROGMEM = "<br/>";
  const char HTTP_ITEM            [] PROGMEM = "<div><a href='#p' onclick='c(this)'>{v}</a><div role='img' class='q q-{r} {l}'></div></div>";
  const char HTTP_RADIO           [] PROGMEM = "<input style='display: inline-block;' type='radio' id='{i}' name='{n}' value='{v}' {c}><label for='{i}'>{1}</label><br/>";
        // <input style='display: inline-block;' type='radio' id='choice2' name='program_selection' value='2'>
        // <label for='choice2'>Choice2</label><br>  
  const char HTTP_FORM_CM_START   [] PROGMEM = "<form method='get' action='ms'>";
  const char HTTP_FORM_CM_END     [] PROGMEM = "<br/><button type='submit'>save</button></form>";
  const char HTTP_BT              [] PROGMEM = "<form action='/{i}' method='get'><button>{n}</button></form>";
  // Info html
  // @todo remove html elements from progmem, repetetive strings
  #ifdef ESP32
  const char HTTP_INFO_esphead    [] PROGMEM = "<h3>esp32</h3><hr><dl>";
  const char HTTP_INFO_chiprev    [] PROGMEM = "<dt>Chip Rev</dt><dd>{1}</dd>";
  const char HTTP_INFO_lastreset  [] PROGMEM = "<dt>Last reset reason</dt><dd>CPU0: {1}<br/>CPU1: {2}</dd>";
  const char HTTP_INFO_aphost     [] PROGMEM = "<dt>Access Point Hostname</dt><dd>{1}</dd>";
  const char HTTP_INFO_psrsize    [] PROGMEM = "<dt>PSRAM Size</dt><dd>{1} bytes</dd>";
  const char HTTP_INFO_temp       [] PROGMEM = "<dt>Temperature</dt><dd>{1} C&deg; / {2} F&deg;</dd><dt>Hall</dt><dd>{3}</dd>";
  #else
  const char HTTP_INFO_esphead    [] PROGMEM = "<h3>esp8266</h3><hr><dl>";
  const char HTTP_INFO_fchipid    [] PROGMEM = "<dt>Flash Chip ID</dt><dd>{1}</dd>";
  const char HTTP_INFO_corever    [] PROGMEM = "<dt>Core Version</dt><dd>{1}</dd>";
  const char HTTP_INFO_bootver    [] PROGMEM = "<dt>Boot Version</dt><dd>{1}</dd>";
  const char HTTP_INFO_lastreset  [] PROGMEM = "<dt>Last reset reason</dt><dd>{1}</dd>";
  const char HTTP_INFO_flashsize  [] PROGMEM = "<dt>Real Flash Size</dt><dd>{1} bytes</dd>";
  #endif

  const char HTTP_INFO_memsmeter  [] PROGMEM = "<br/><progress value='{1}' max='{2}'></progress></dd>";
  const char HTTP_INFO_memsketch  [] PROGMEM = "<dt>Memory - Sketch Size</dt><dd>Used / Total bytes<br/>{1} / {2}";
  const char HTTP_INFO_freeheap   [] PROGMEM = "<dt>Memory - Free Heap</dt><dd>{1} bytes available</dd>";
  const char HTTP_INFO_wifihead   [] PROGMEM = "<br/><h3>WiFi</h3><hr>";
  const char HTTP_INFO_uptime     [] PROGMEM = "<dt>Uptime</dt><dd>{1} Mins {2} Secs</dd>";
  const char HTTP_INFO_chipid     [] PROGMEM = "<dt>Chip ID</dt><dd>{1}</dd>";
  const char HTTP_INFO_idesize    [] PROGMEM = "<dt>Flash Size</dt><dd>{1} bytes</dd>";
  const char HTTP_INFO_sdkver     [] PROGMEM = "<dt>SDK Version</dt><dd>{1}</dd>";
  const char HTTP_INFO_cpufreq    [] PROGMEM = "<dt>CPU Frequency</dt><dd>{1}MHz</dd>";
  const char HTTP_INFO_apip       [] PROGMEM = "<dt>Access Point IP</dt><dd>{1}</dd>";
  const char HTTP_INFO_apmac      [] PROGMEM = "<dt>Access Point MAC</dt><dd>{1}</dd>";
  const char HTTP_INFO_apssid     [] PROGMEM = "<dt>Access Point SSID</dt><dd>{1}</dd>";
  const char HTTP_INFO_apbssid    [] PROGMEM = "<dt>BSSID</dt><dd>{1}</dd>";
  const char HTTP_INFO_stassid    [] PROGMEM = "<dt>Station SSID</dt><dd>{1}</dd>";
  const char HTTP_INFO_staip      [] PROGMEM = "<dt>Station IP</dt><dd>{1}</dd>";
  const char HTTP_INFO_stagw      [] PROGMEM = "<dt>Station Gateway</dt><dd>{1}</dd>";
  const char HTTP_INFO_stasub     [] PROGMEM = "<dt>Station Subnet</dt><dd>{1}</dd>";
  const char HTTP_INFO_dnss       [] PROGMEM = "<dt>DNS Server</dt><dd>{1}</dd>";
  const char HTTP_INFO_host       [] PROGMEM = "<dt>Hostname</dt><dd>{1}</dd>";
  const char HTTP_INFO_stamac     [] PROGMEM = "<dt>Station MAC</dt><dd>{1}</dd>";
  const char HTTP_INFO_conx       [] PROGMEM = "<dt>Connected</dt><dd>{1}</dd>";
  const char HTTP_INFO_autoconx   [] PROGMEM = "<dt>Autoconnect</dt><dd>{1}</dd>";
  const char HTTP_INFO_ap         [] PROGMEM = "Point d'accès";
  const char HTTP_INFO_sta        [] PROGMEM = "Station";
  const char HTTP_INFO_con_1      [] PROGMEM = "<dt>La connexion est définie comme:</dt><dd style=\"padding:0 0 0 5px;\">{1}</dd>";
  const char HTTP_INFO_con_3      [] PROGMEM = "<dt>Vous etes connecter au router:</dt><dd style=\"padding:0 0 0 5px;\">{1}</dd>";
  const char HTTP_INFO_con_2      [] PROGMEM = "<dt>Pour vous connecter à l'appareil, Connecter vous au routeur indiquer ci dessus, pui saisir l'adresse IP ci-dessous dans les réglages IP de l'application android:</dt><dd style=\"padding:0 0 0 5px;\">{1}</dd>";

  const char HTTP_INFO_aboutver   [] PROGMEM = "<dt>WiFiManager</dt><dd>{1}</dd>";
  const char HTTP_INFO_aboutarduino[] PROGMEM = "<dt>Arduino</dt><dd>{1}</dd>";
  const char HTTP_INFO_aboutsdk   [] PROGMEM = "<dt>ESP-SDK/IDF</dt><dd>{1}</dd>";
  const char HTTP_INFO_aboutdate  [] PROGMEM = "<dt>Build Date</dt><dd>{1}</dd>";

  const char S_y                  [] PROGMEM = "Yes";
  const char S_n                  [] PROGMEM = "No";
  const char S_enable             [] PROGMEM = "Enabled";
  const char S_disable            [] PROGMEM = "Disabled";

  const char T_1                  [] PROGMEM = "{1}"; // @token 1
  const char T_2                  [] PROGMEM = "{2}"; // @token 2
  const char T_3                  [] PROGMEM = "{3}"; // @token 2  

  // http
  const char HTTP_HEAD_CL[]         PROGMEM = "Content-Length";
  const char HTTP_HEAD_CT[]         PROGMEM = "text/html";
  const char HTTP_HEAD_CT2[]        PROGMEM = "text/plain";
  const char HTTP_HEAD_CORS[]       PROGMEM = "Access-Control-Allow-Origin";
  const char HTTP_HEAD_CORS_ALLOW_ALL[]  PROGMEM = "*";  

  #endif

  // endregion >>>> WIFIMANAGER - 


  class WiFiManagerCPY {
    bool          _cleanConnect   = true;   // disconnect before connect in connectwifi, increases stability on connects
    boolean       storeSTAmode    = true;   // option store persistent STA mode in connectwifi 
    bool          esp32persistent = false;

    bool          WiFi_Mode(WiFiMode_t m);
    bool          WiFi_Mode(WiFiMode_t m,bool persistent);
    bool          WiFi_enableSTA(bool enable,bool persistent);
    bool          WiFi_eraseConfig();
    uint8_t       WiFi_softap_num_stations();
    bool          WiFi_hasAutoConnect();
    String        WiFi_psk(bool persistent = true) const;

    String        getWLStatusString(uint8_t status);
    String        getModeString(uint8_t status);

  public:
    String        WiFi_SSID(bool persistent = true) const;
    String        toStringIp(IPAddress ip);
    boolean       isIp(String str);

    bool          WiFi_Disconnect();
    bool          WiFi_enableSTA(bool enable);

    void          connectWifi(String ssid, String pass, bool connect = true);
    bool          wifiConnectNew(String ssid, String pass,bool connect = true);
    bool          wifiConnectDefault();

    WiFiManagerCPY(){};
    
  };

#endif // _WCEVO_WIFIMANAGER_H