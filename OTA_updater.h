#if !defined OTAUPDATER_H
#define OTAUPDATER_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include "config.h"
#include <WiFiClient.h>

#if HW_PLATFORM == 0
# include <ESP8266WebServer.h>
# include <ESP8266mDNS.h>
#elif HW_PLATFORM == 1
# include <WebServer.h>
# include <ESPmDNS.h>
# include <Update.h>
#endif


class OTAUpdater
{

private: 
 

public: 

  //virtual OTAUpdater() = 0;
  virtual void OTA_handle() = 0;
  virtual void begin() = 0;
  
};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
