#if !defined COMMUNICATIONHANDLER_H
#define COMMUNICATIONHANDLER_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "config.h"
#include "common_datatypes.h"
#include "timeSync.h"

class CommunicationHandler
{

    

  public:

    lamp_status* m_lamp_status_request;
    COMM_TYPE m_communication_type;
    timeSync* m_timer;

    CommunicationHandler(lamp_status* lamp_status_request, COMM_TYPE communication_type,timeSync* timer):
    m_lamp_status_request(lamp_status_request),
    m_communication_type(communication_type),
    m_timer(timer)
    {
       
    };

    virtual void begin() = 0;
    virtual void stop() = 0;
    virtual void configure() = 0;
    virtual void network_loop() = 0;   
    virtual void publish_initcomm() {};
    virtual void finish_initcomm() {};

    bool is_targeted_device(uint8_t req_id_mask, uint8_t deviceID)
    {
      uint8_t device_mask = 1 << deviceID;

      if ( (device_mask & req_id_mask) != 0u ) return true;
      else return false;
    }

    COMM_TYPE get_comm_type()
    {
      return  m_communication_type;
    }

};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
