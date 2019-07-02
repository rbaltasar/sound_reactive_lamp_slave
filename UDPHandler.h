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
  uint8_t m_message[50];
  lamp_status m_lamp_status_request_local;
  bool received_msg;
  bool received_mode_select;
  
  void synchronize(unsigned long delay_ms);
  UDP_Message_Id get_msg_id(uint8_t msgID);
  void process_message();

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
