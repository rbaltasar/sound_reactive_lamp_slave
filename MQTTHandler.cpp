
#include "MQTTHandler.h"

/* Constructor. The object is created ony once */
MQTTHandler::MQTTHandler(lamp_status* lamp_status_request,timeSync* timer):
CommunicationHandler(lamp_status_request,MQTT,timer),
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

/* Start communication */
void MQTTHandler::begin() 
{
  /* Subscribe to topics */
  Serial.println("Subscribing to topics");
  subscribe_topics();

  /* Restet alive timer to avoid reboot due to communication error */
  m_last_alive_rx = millis();
 
}

/* Stop communication. TODO: unsubscribe? */
void MQTTHandler::stop()
{
  //Nothing to do
  Serial.println("Stopping MQTT communication handler");
}

/* Configure the callback function for a subscribed MQTT topic */
void MQTTHandler::callback(char* topic, byte* payload, unsigned int length) {

  /* Print message (debugging only) */
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  /* Parse JSON object */
  JsonObject& root = jsonBuffer.parseObject(payload);

  /* Filter for topics */
  /* Mode request */
  if( strcmp(topic,"lamp_network/mode_request") == 0 )
  {
    /* Check if this message is targeted for this node */
    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    /* Update shared memory */
    m_lamp_status_request->lamp_mode = root["mode"];
    Serial.print("Lamp mode requested: ");
    Serial.println(m_lamp_status_request->lamp_mode);
  }
  /* Brightness request */
  else if(strcmp(topic,"lamp_network/light_intensity") == 0)
  {
    /* Check if this message is targeted for this node */
    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    int rcv = root["intensity"];
    
    /* Value conversion. TODO: improve */
    if(rcv == 0) rcv = 255;    
    else rcv = 11 - rcv;    
    
    /* Update shared memory */
    m_lamp_status_request->brightness = rcv;
    Serial.print("Brightness requested: ");
    Serial.println(rcv);
  }

  /* Effect delay request */
  else if(strcmp(topic,"lamp_network/effect_delay") == 0)
  {
    /* Check if this message is targeted for this node */
    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    int rcv = root["delay"];
    /* Update shared memory */
    m_lamp_status_request->effect_amount = rcv;    
    m_lamp_status_request->effect_delay = rcv * 10; //Delay in ms. TODO: improve conversion
    Serial.print("Delay requested: ");
    Serial.println(rcv);
  }
  
  /* Effect speed request */
  else if(strcmp(topic,"lamp_network/effect_speed") == 0)
  {
    /* Check if this message is targeted for this node */
    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    /* Value conversion. TODO: improve */
    int rcv = root["speed"];
    rcv = 1000 - 10 * rcv;
    /* Update shared memory */
    m_lamp_status_request->effect_speed = rcv; //Delay in ms
    Serial.print("Effect speed requested: ");
    Serial.println(rcv);
  }  

  /* Light color request */
  else if(strcmp(topic,"lamp_network/light_color") == 0)
  {
    /* Check if this message is targeted for this node */
    if(!is_targeted_device(root["id_mask"],m_lamp_status_request->deviceID)) return;
    
    /* Update shared memory */
    m_lamp_status_request->color.R = root["R"];
    m_lamp_status_request->color.G = root["G"];
    m_lamp_status_request->color.B = root["B"];

    // Output to serial monitor
    Serial.print("Color requested: ");
    Serial.println(m_lamp_status_request->color.R);
    Serial.println(m_lamp_status_request->color.G);
    Serial.println(m_lamp_status_request->color.B);
  }
  
  /* Light amount information */
  else if(strcmp(topic,"livingroom_node/light") == 0)
  {
    /* Update shared memory */
    m_lamp_status_request->light_amount = root["light"];
    Serial.print("Received light amount: ");
    Serial.println(m_lamp_status_request->light_amount);
  }

  /* Initial communication handshake response */
  else if(strcmp(topic,"lamp_network/initcommrx") == 0)
  {
    /* Check if the handshake is addressed to this node. Before ID assignation the MAC address is the unique identifier */
    const char* mac_request = root["mac"];
    if(strcmp(mac_request,m_lamp_status_request->MACAddress_string.c_str()) == 0)
    {
      /* Update shared memory */
      m_lamp_status_request->deviceID = root["id"];
      String ota_url = "lamp" + String(m_lamp_status_request->deviceID + 1);
      m_lamp_status_request->ota_url = ota_url.c_str();//root["url"];
      m_lamp_status_request->lamp_mode = root["mode"];      
      m_lamp_status_request->initState.isCompleted = true;

      Serial.println(m_lamp_status_request->ota_url);
    }     
  }
  
  /* Alive check topic */
  else if(strcmp(topic,"lamp_network/alive_rx") == 0)
  {
    /* Update last alive check received to keep communication alive */
    m_last_alive_rx = millis();
  }
}

/* Communication handling loop */
void MQTTHandler::network_loop()
{
  /* MQTT loop */
  if (!m_client.connected()) reconnect();
  m_client.loop();

  unsigned long now = millis(); 

  /* Publish alive message */
  if( (now - m_last_alive_tx)> ALIVE_PERIOD)
  {   
    m_client.publish("lamp_network/alive_tx", String(m_lamp_status_request->deviceID).c_str());
    m_last_alive_tx = now;
  }

  /* Reset controlelr if communication is lost (no alive message received in defined time) */
  if( (now - m_last_alive_rx)> (3*ALIVE_PERIOD))
  {   
    Serial.println("Lost MQTT connection. Reboot");
    ESP.restart();
  }
}

/* Subscribe to all the topics from the topic list */
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

      /* Restart the microcontroller after 10 failed attempts to connect */
      if(++m_mqtt_reconnect_counter > 10)  ESP.restart();
    }
  }  
}

/* Publish initial communication handshake */
void MQTTHandler::publish_initcomm()
{

  Serial.println("Starting communication handshake");
  
  m_client.subscribe("lamp_network/initcommrx");

  m_last_alive_rx = millis();
  
  StaticJsonBuffer<256> jsonBuffer_send;
  JsonObject& root_send = jsonBuffer_send.createObject();

  /* Send information about our MAC address, IP and reset reason */
  root_send["mac"] = m_lamp_status_request->MACAddress_string.c_str();
  root_send["ip"] = m_lamp_status_request->IPAddress_string.c_str();
  root_send["rst_0"] = "0";
  root_send["rst_1"] = "0";

  char JSONmessageBuffer[256];
  root_send.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));  
  m_client.publish("lamp_network/initcomm_tx", JSONmessageBuffer); 
}

/* Finish communication handshake */
void MQTTHandler::finish_initcomm()
{
  /* Unsubscribe to communication handshake topic */
  m_client.unsubscribe("lamp_network/initcommrx");

  /* Begin the normal communication */
  begin();
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
