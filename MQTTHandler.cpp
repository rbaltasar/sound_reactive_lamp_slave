
#include "MQTTHandler.h"

MQTTHandler::MQTTHandler(lamp_status* lamp_status_request):
CommunicationHandler(lamp_status_request,MQTT),
jsonBuffer(250),
m_mqtt_reconnect_counter(0),
m_last_alive_tx(0),
m_last_alive_rx(0)
{
  /* Set client */
  m_client.setClient(espClient);
  /* Define MQTT broker */
  m_client.setServer(mqtt_server, 1883);
  /* Define callback function */
  m_client.setCallback(std::bind(&MQTTHandler::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void MQTTHandler::begin() 
{
  /* Subscribe to topics */
  subscribe_topics(); 
}

void MQTTHandler::stop()
{

}

/* Configure the callback function for a subscribed MQTT topic */
void MQTTHandler::callback(char* topic, byte* payload, unsigned int length) {

  /* Print message (debugging only) */
#if 1
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif

  /* Parse JSON object */
  JsonObject& root = jsonBuffer.parseObject(payload);

  /* Filter for topics */
  if( strcmp(topic,"lamp_network/mode_request") == 0 )
  {
    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    m_lamp_status_request->lamp_mode = root["mode"];   
    Serial.println(m_lamp_status_request->lamp_mode);
  }
  
  else if(strcmp(topic,"lamp_network/light_intensity") == 0)
  {

    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    int rcv = root["intensity"];

    if(rcv == 0) rcv = 255;
    
    else
    {
      rcv = 11 - rcv;
    }
    
    m_lamp_status_request->brightness = rcv;
    Serial.println(rcv);
  }

  else if(strcmp(topic,"lamp_network/effect_delay") == 0)
  {

    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    int rcv = root["delay"];
    m_lamp_status_request->effect_amount = rcv;
    rcv = rcv * 10;
    m_lamp_status_request->effect_delay = rcv; //Delay in ms
    Serial.println(rcv);
  }

  else if(strcmp(topic,"lamp_network/effect_speed") == 0)
  {
    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    int rcv = root["speed"];
    rcv = 1000 - 10 * rcv;
    m_lamp_status_request->effect_speed = rcv; //Delay in ms
    Serial.println(rcv);
  }  

  else if(strcmp(topic,"lamp_network/light_color") == 0)
  {
    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    m_lamp_status_request->color.R = root["R"];
    m_lamp_status_request->color.G = root["G"];
    m_lamp_status_request->color.B = root["B"];

    // Output to serial monitor
#if 1
    Serial.println(m_lamp_status_request->color.R);
    Serial.println(m_lamp_status_request->color.G);
    Serial.println(m_lamp_status_request->color.B);
#endif
  }

  else if(strcmp(topic,"livingroom_node/light") == 0)
  {
    m_lamp_status_request->light_amount = root["light"];
    Serial.print("Received light amount: ");
    Serial.println(m_lamp_status_request->light_amount);
  }

  else if(strcmp(topic,"lamp_network/initcommrx") == 0)
  {
    const char* mac_request = root["mac_origin"];

    if(strcmp(mac_request,m_lamp_status_request->MACAddress_string.c_str()) == 0)
    {
      m_lamp_status_request->deviceID = root["deviceID"];
      m_lamp_status_request->ota_url = root["OTA_URL"];
      m_lamp_status_request->lamp_mode = root["mode"];    
    }
    
    m_lamp_status_request->initState.isCompleted = true;
  }
  else if(strcmp(topic,"lamp_network/alive_rx") == 0)
  {
    m_last_alive_rx = millis();
  }
}

void MQTTHandler::network_loop()
{
  /* MQTT loop */
  if (!m_client.connected()) reconnect();
  m_client.loop();

  unsigned long now = millis(); 

  if( (now - m_last_alive_tx)> ALIVE_PERIOD)
  {   
    m_client.publish("lamp_network/alive_tx", String(m_lamp_status_request->deviceID).c_str());
    m_last_alive_tx = now;
  }

  if( (now - m_last_alive_rx)> (3*ALIVE_PERIOD))
  {   
    Serial.println("Lost MQTT connection. Reboot");
    ESP.restart();
  }
}


void MQTTHandler::subscribe_topics()
{
  for(uint8_t i = 0; i < NUM_SUBSCRIBED_TOPICS; i++)
  {
    m_client.subscribe(topic_subscribe_list[i].c_str());
  }
}


/* Reconnect to the MQTT broker */
void MQTTHandler::reconnect()
{
  // Loop until we're reconnected
  if (!m_client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (m_client.connect(m_lamp_status_request->IPAddress_string.c_str())) //Unique name for each instance of a slave
    {
      Serial.println("connected");
      //Resubscribe
      subscribe_topics();
      m_mqtt_reconnect_counter = 0;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(m_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(500);

      if(++m_mqtt_reconnect_counter > 10)  ESP.restart();
    }
  }  
}

void MQTTHandler::publish_initcomm()
{
  StaticJsonBuffer<256> jsonBuffer_send;
  JsonObject& root_send = jsonBuffer_send.createObject();

  root_send["mac"] = m_lamp_status_request->MACAddress_string.c_str();
  root_send["ip"] = m_lamp_status_request->IPAddress_string.c_str();
  root_send["rst_0"] = "0";
  root_send["rst_1"] = "0";

  char JSONmessageBuffer[256];
  root_send.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));  
  m_client.publish("lamp_network/initcomm_tx", JSONmessageBuffer); 
}

void MQTTHandler::finish_initcomm()
{
  m_client.unsubscribe("lamp_network/initcommrx");
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
