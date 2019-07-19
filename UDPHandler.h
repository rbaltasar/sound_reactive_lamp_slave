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

  /* UDP client */
  AsyncUDP m_UDP;
  /* Intermediate buffer for fast callback handling */
  uint8_t m_message[6 * NUM_MAX_LAMPS];
  /* Local shadow of lamp status for intermediate buffering */
  lamp_status m_lamp_status_request_local;
  /* Local states */
  bool m_received_msg, m_received_mode_select, m_received_config, m_received_color_payload;
  /* Alive check timestamps */
  unsigned long m_last_alive_tx, m_last_alive_rx;

  /* Private functions */
  void synchronize(unsigned long delay_ms);
  UDP_Message_Id get_msg_id(uint8_t msgID);
  void process_message();
  void send_alive();
  uint8_t compute_amplitude(uint8_t relative_amplitude);

public: 

  UDPHandler(lamp_status* lamp_status_request,timeSync* timer);
  ~UDPHandler();

  void begin();
  void stop();
  void configure(){}; //Nothing to do but overload needed
  void network_loop();

 
};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
