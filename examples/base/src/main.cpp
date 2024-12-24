#include <altoolslib.h>
#include <wificonnectevo.h>


AsyncWebServer  webserver(80);
DNSServer       dnsServer;
WCEVO_manager   _WCEVO_manager("ex_wcevo", "wcevo1234", &dnsServer, &webserver);  


void setup() {

  Serial.begin(115200);

  for(unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime > 5000); ) { }
  delay(3000);

  Serial.println();
  Serial.printf_P(PSTR("\n#############\nEX WCEVO\n#############\n\n"));
  
  #ifdef ALT_DEBUG_TARCE
    ALT_debugBuffer = new char[1024];  
    _DebugPrintList.add("main");  
    _DebugPrintList.add(WCEVO_DEBUGREGION_WCEVO);  
    _DebugPrintList.add(WCEVO_DEBUGREGION_AP);  
    _DebugPrintList.add(WCEVO_DEBUGREGION_STA);  
  #endif 


  WCEVO_managerPtrGet()->set_credential("free-3C3786-EXT", "SSIDPASS");
  _WCEVO_manager.set_cm(WCEVO_CM_STAAP);
  _WCEVO_manager.set_cmFail(WCEVO_CF_RESET);
  _WCEVO_manager.start();
  _WCEVO_manager.print();

}

void loop() {
  _WCEVO_manager.handleConnection();
  _Sr_menu.serialRead();
}


