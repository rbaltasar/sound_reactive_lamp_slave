#include "UDPHandler.h" 

UDPHandler::UDPHandler(lamp_status* lamp_status_request,timeSync* timer):
CommunicationHandler(lamp_status_request,UDP,timer),
received_mode_select(false),
received_msg(false),
m_received_config(false),
m_received_color_payload(false)
{
 
}

UDPHandler::~UDPHandler()
{

}

UDP_Message_Id UDPHandler::get_msg_id(uint8_t msgID)
{
  if(msgID == 0) return MODE_SELECT;
  else if(msgID == 1) return SYNC_REQ;
  else if(msgID == 2) return ACK;
  else if(msgID == 4) return PAYLOAD_SINGLE;
  else if(msgID == 5) return PAYLOAD_WINDOW;
  else if(msgID == 6) return PAYLOAD_FULL;
  else if(msgID == 7) return CONFIGURATION;
  
  else return ERR;  
}

void UDPHandler::begin() 
{
  Serial.println("Starting UDP communication handler");
 
  /* Ignore the first msg after begin --> may be corrupted data in UDP buffer --> prevent switch to MQTT */
  received_msg = false;
  received_mode_select = false;
  m_received_color_payload = false;
  m_received_config = false;

  /* Default effect delay */
  m_lamp_status_request->effect_delay = 30;

  if(m_UDP.listenMulticast(IPAddress(239,1,2,3), 7001)) {

        m_UDP.onPacket([this](AsyncUDPPacket packet) {

            noInterrupts();
            memcpy(m_message, packet.data(), packet.length());
            received_msg = true;          
            interrupts();

        });       
    }
}

/* Convert a relative amplitude in an interval [0-100] to an absolute amplitude
 * proportional to the number of leds
 */
uint8_t UDPHandler::compute_amplitude(uint8_t relative_amplitude)
{
  float amplitude = (float)relative_amplitude / 100.0;

  amplitude *= NUM_LEDS;

  return (uint8_t)amplitude;
}

void UDPHandler::process_message()
{
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

      m_lamp_status_request_local.resync = true;

      Serial.print("Received Sync Request: ");
      Serial.println(msg_struct.delay_ms);
 
        
      break;
    }
    case CONFIGURATION:
    {
      
      udp_music_mode_configuration msg_struct = *((udp_music_mode_configuration*)m_message);

      m_lamp_status_request_local.effect_delay = msg_struct.effect_delay;
      m_lamp_status_request_local.effect_direction = msg_struct.effect_direction;
      m_lamp_status_request_local.color.R = msg_struct.base_color.R;
      m_lamp_status_request_local.color.G = msg_struct.base_color.G;
      m_lamp_status_request_local.color.B = msg_struct.base_color.B;
      m_lamp_status_request_local.color_increment = msg_struct.color_increment;
      
      m_received_config = true;
      
      Serial.print("Received configuration message: ");
      Serial.println(msg_struct.effect_delay);
      Serial.println(msg_struct.effect_direction);
      Serial.println(msg_struct.base_color.R);
      Serial.println(msg_struct.base_color.G);
      Serial.println(msg_struct.base_color.B);
      Serial.println(msg_struct.color_increment);
        
      break;
    }
    case PAYLOAD_SINGLE:
    {
      udp_payload_msg msg_struct = *((udp_payload_msg*)m_message);

      /* Check if this message is targeted for this node */
      if(!is_targeted_device(msg_struct.payload.mask,m_lamp_status_request->deviceID))
      {
        /* With single payload, a message addressed to this node is discarded
         * The effect must be zero
         */
        m_lamp_status_request_local.color.R = 0;
        m_lamp_status_request_local.color.G = 0;
        m_lamp_status_request_local.color.B = 0;
        m_lamp_status_request_local.amplitude = 0;
      }
      else
      {
        /* Do not update the color if the selected effect is ENERGY BAR COLOR */
        if(m_lamp_status_request->lamp_mode != (100 + ENERGY_BAR_COLOR))
        {
          m_lamp_status_request_local.color.R = msg_struct.payload.color.R;
          m_lamp_status_request_local.color.G = msg_struct.payload.color.G;
          m_lamp_status_request_local.color.B = msg_struct.payload.color.B;
        }
        
        m_lamp_status_request_local.amplitude = compute_amplitude(msg_struct.payload.amplitude);
      }

#if 0
      Serial.print("Received Single Payload msg: [");
      Serial.print(msg_struct.payload.color.R);
      Serial.print("][");
      Serial.print(msg_struct.payload.color.G);
      Serial.print("][");
      Serial.print(msg_struct.payload.color.B);
      Serial.print("][");
      Serial.print(msg_struct.payload.amplitude);
      Serial.println("]");
#endif

      m_received_color_payload = true;
        
      break;
    }
    case PAYLOAD_WINDOW:
    {
      udp_payload_window_spectrum_msg msg_struct = (*(udp_payload_window_spectrum_msg*)m_message); 

      Serial.println("Received Window Payload msg:");
      Serial.print("Size: ");
      Serial.println(msg_struct.numMsg);

      /* Loop over all received frequency windows */
      for(uint8_t i=0; i < msg_struct.numMsg; i++)
      {        
        /* Filter the message addressed to this node */
        if(is_targeted_device(msg_struct.payload[i].mask,m_lamp_status_request->deviceID))
        {
          Serial.print("Message addressed to this node: ");
          Serial.print("R: ");
          Serial.print(msg_struct.payload[i].color.R);
          Serial.print(" G: ");
          Serial.print(msg_struct.payload[i].color.G);
          Serial.print(" B: ");
          Serial.print(msg_struct.payload[i].color.B);
          Serial.print(" Ampl: ");
          Serial.print(msg_struct.payload[i].amplitude);
          Serial.print(" Mask: ");
          Serial.println(msg_struct.payload[i].mask);

          /* Do not update the color if the selected effect is ENERGY BAR COLOR */
          if(m_lamp_status_request->lamp_mode != (100 + ENERGY_BAR_COLOR))
          {
            m_lamp_status_request_local.color.R = msg_struct.payload[i].color.R;
            m_lamp_status_request_local.color.G = msg_struct.payload[i].color.G;
            m_lamp_status_request_local.color.B = msg_struct.payload[i].color.B;
          }
          
          m_lamp_status_request_local.amplitude = compute_amplitude(msg_struct.payload[i].amplitude);

          m_received_color_payload = true;

          /* Do not continue searching */
          break;          
        }        
      }      
        
      break;
    }
    case PAYLOAD_FULL:
    {
      udp_payload_full_spectrum_msg msg_struct = *((udp_payload_full_spectrum_msg*)m_message);

      Serial.print("Received Full Payload msg: [");
      Serial.println("]");
        
      break;
    }
    case ACK:
    default:
      break;                 
  }

  received_msg = false;

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

  if(received_msg)
  {
    noInterrupts();
    process_message();
    interrupts();
  }

  /* Synchronous update of the shared memory */
  //Serial.println("Network loop UDP!");
  if(m_received_color_payload)
  {
    m_lamp_status_request->color.R = m_lamp_status_request_local.color.R;
    m_lamp_status_request->color.G = m_lamp_status_request_local.color.G;
    m_lamp_status_request->color.B = m_lamp_status_request_local.color.B;
    m_lamp_status_request->amplitude = m_lamp_status_request_local.amplitude;
    m_lamp_status_request->new_payload = true;

    m_received_color_payload = false;
  }
 
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
  if(m_received_config)
  {
    m_lamp_status_request->effect_delay = m_lamp_status_request_local.effect_delay;
    m_lamp_status_request->effect_direction = m_lamp_status_request_local.effect_direction;
    m_lamp_status_request->color_increment = m_lamp_status_request_local.color_increment;
    m_received_config = false;
  }

}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
