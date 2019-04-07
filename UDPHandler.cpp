
#include "UDPHandler.h"
 

UDPHandler::UDPHandler(lamp_status* lamp_status_request):
m_lamp_status_request(lamp_status_request)
{
  
}

void UDPHandler::begin() 
{
  m_UDP.begin(7001);
}

void UDPHandler::stop()
{
  m_UDP.stop();
}

void UDPHandler::synchronize(unsigned long delay_ms)
{ 
  Serial.print("Synchronized with delay: ");
  Serial.println(delay_ms);

  /* Delay synchronization */
  delay(delay_ms);

  /* Acknowledge synchronization */
  uint8_t resp = 3;
  m_UDP.beginPacket(masterIP,7001);
  m_UDP.write(resp);
  m_UDP.endPacket();  

  /* Indicate that resynchronization has been done */
  m_lamp_status_request->resync = true;
  
}

bool UDPHandler::network_loop()
{

  bool retval = true;
  
  int packetSize = m_UDP.parsePacket();
  if (packetSize)
  {
    /* Read the packet */
    m_UDP.read((char *)&m_message, packetSize);

    /* Sync request */
    if(m_message[0] == 0x02)
    {
      sync_request req = *((sync_request*)m_message);      

      /* Do synchronization */
      synchronize((unsigned long)req.msgContent);

      Serial.print("Message type: ");
      Serial.println(req.msgID);
      Serial.print("Message content: ");
      Serial.println(req.msgContent);

    }   
    /* Streaming message (color & amplitude) */
    else if(m_message[0] == 0x01)
    {
      color_request req = *((color_request*)m_message);      

      /* Select color for next prints */
      m_lamp_status_request->color.R = req.red;
      m_lamp_status_request->color.G = req.green;
      m_lamp_status_request->color.B = req.blue;
    }

     /* Stop streaming message */
    else if(m_message[0] == 0x03)
    {
      /* Answer with an acknowledge */

      /* Set return value to stop streaming mode */
      retval = false;
    }
  }

  return retval;
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
