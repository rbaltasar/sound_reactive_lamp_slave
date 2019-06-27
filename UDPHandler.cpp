#include "UDPHandler.h" 

UDPHandler::UDPHandler(lamp_status* lamp_status_request,timeSync* timer):
CommunicationHandler(lamp_status_request,UDP,timer),
received_mode_select(false)
{
 
}

UDPHandler::~UDPHandler()
{

}

UDP_Message_Id UDPHandler::get_msg_id(uint8_t msgID)
{
  if(msgID == 0) return MODE_SELECT;
  else if(msgID == 1) return SYNC_REQ;
  else if(msgID == 2) return PAYLOAD;
  else return ERR;  
}

void UDPHandler::begin() 
{
  Serial.println("Starting UDP communication handler");

  if(m_UDP.listenMulticast(IPAddress(239,1,2,3), 7001)) {

        m_UDP.onPacket([this](AsyncUDPPacket packet) {

            m_message = packet.data();

            switch(get_msg_id(m_message[0]))
            {
              case MODE_SELECT:
              {
                
                udp_mode_select msg_struct = *((udp_mode_select*)m_message);

                m_lamp_status_request_local.lamp_mode = msg_struct.mode_select;

                received_mode_select = true;
                
                Serial.print("Received Mode Select: ");
                Serial.println(msg_struct.mode_select);
                  
                break;
              }
              case SYNC_REQ:
              {
                udp_sync_req msg_struct = *((udp_sync_req*)m_message);

                //synchronize(msg_struct.delay_ms);

                m_lamp_status_request_local.resync = true;

                Serial.print("Received Sync Request: ");
                Serial.println(msg_struct.delay_ms);
                  
                break;
              }
              case PAYLOAD:
              {
                udp_payload msg_struct = *((udp_payload*)m_message);

                /* Check if this message is targeted for this node */
                if(!is_targeted_device(msg_struct.mask,m_lamp_status_request->deviceID))
                {
                  m_lamp_status_request_local.color.R = 0;
                  m_lamp_status_request_local.color.G = 0;
                  m_lamp_status_request_local.color.B = 0;
                  m_lamp_status_request_local.amplitude = 1;
                }
                else
                {
                  m_lamp_status_request_local.color.R = msg_struct.red;
                  m_lamp_status_request_local.color.G = msg_struct.green;
                  m_lamp_status_request_local.color.B = msg_struct.blue;
                  m_lamp_status_request_local.amplitude = msg_struct.amplitude;
                }

                Serial.print("Received Payload: [");
                Serial.print(msg_struct.red);
                Serial.print("][");
                Serial.print(msg_struct.green);
                Serial.print("][");
                Serial.print(msg_struct.blue);
                Serial.print("][");
                Serial.print(msg_struct.amplitude);
                Serial.println("]");
                  
                break;
              }
              case ACK:
              default:
                break;                 
            }

        });       
    }
}

void UDPHandler::stop()
{
  Serial.println("Stopping UDP communication handler");
  m_UDP.close();
}

void UDPHandler::synchronize(unsigned long delay_ms)
{ 
  Serial.print("Synchronized with delay: ");
  Serial.println(delay_ms);

  /* Delay synchronization */
  delay(delay_ms);

}

void UDPHandler::network_loop()
{

  /* Synchronous update of the shared memory */
  //Serial.println("Network loop UDP!");
  m_lamp_status_request->color.R = m_lamp_status_request_local.color.R;
  m_lamp_status_request->color.G = m_lamp_status_request_local.color.G;
  m_lamp_status_request->color.B = m_lamp_status_request_local.color.B;
  m_lamp_status_request->amplitude = m_lamp_status_request_local.amplitude;
  if(received_mode_select)
  {
    m_lamp_status_request->lamp_mode = m_lamp_status_request_local.lamp_mode;
    received_mode_select = false;
    Serial.println("Updating shared mode");
    Serial.println(m_lamp_status_request->lamp_mode);
  }
  if(m_lamp_status_request_local.resync)
  {
    m_lamp_status_request->resync = true;
    m_lamp_status_request_local.resync = false;
  }

}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
