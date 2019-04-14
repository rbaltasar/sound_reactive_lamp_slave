
#include "OTA_updater.h"
#if HW_PLATFORM == 0

#if !defined OTAUPDATER_ESP12_H
#define OTAUPDATER_ESP12_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif


#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

class OTAUpdater_ESP12: public OTAUpdater
{

private:

  ESP8266WebServer server; 

  const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

public: 

  OTAUpdater_ESP12();
  void OTA_handle();
  void begin(const char* host_name);
  
};

#endif
#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
