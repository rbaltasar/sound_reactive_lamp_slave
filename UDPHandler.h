#if !defined UDPHANDLER_H
#define UDPHANDLER_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include "config.h"
#include "common_datatypes.h"
#include "AsyncUDP.h"
#include "CommunicationHandler.h"

class UDPHandler : public CommunicationHandler
{

private:

  AsyncUDP m_UDP; 
  uint8_t* m_message;
  lamp_status m_lamp_status_request_local;
  
  void synchronize(unsigned long delay_ms);
  UDP_Message_Id get_msg_id(uint8_t msgID);

public: 

  UDPHandler(lamp_status* lamp_status_request,timeSync* timer);
  ~UDPHandler();

  void begin();
  void stop();
  void network_loop();

 
};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
