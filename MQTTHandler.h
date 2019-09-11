#if !defined MQTTHANDLER_H
#define MQTTHANDLER_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "config.h"
#include "common_datatypes.h"
#include "CommunicationHandler.h"

/* List of subscribed topics */
#define NUM_SUBSCRIBED_TOPICS 8

class MQTTHandler : public CommunicationHandler
{

private:

  //DynamicJsonBuffer jsonBuffer;
  WiFiClient espClient;
  PubSubClient m_client;

  uint8_t m_mqtt_reconnect_counter;
  unsigned long m_last_alive_tx;
  unsigned long m_last_alive_rx; 

  /* MQTT settings */
  const char* mqtt_server = "192.168.2.118";

  String topic_subscribe_list[NUM_SUBSCRIBED_TOPICS] = {
    "lamp_network/mode_request",
    "lamp_network/light_intensity",
    "lamp_network/light_color",
    "lamp_network/effect_delay",
    "lamp_network/effect_speed",
    "lamp_network/alive_rx",
    "livingroom_node/light",
    "lamp_network/initcommrx"
  };

  void subscribe_topics();
  void unsubscribe_topics();
  void callback(char* topic, byte* payload, unsigned int length);

public: 

  MQTTHandler(lamp_status* m_lamp_status_request,timeSync* timer);

  void begin();
  void stop();
  void configure();
  void reconnect();
  void network_loop();
  void publish_initcomm();
  void finish_initcomm();
  

 
};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
