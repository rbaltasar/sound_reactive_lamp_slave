
#include "timeSync.h"

#if HW_PLATFORM == 0
# include <ESP8266WiFi.h>
# include "OTA_updater_ESP12.h"
#elif HW_PLATFORM == 1
# include <WiFi.h>
# include "OTA_updater_ESP32.h"
#endif

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "LED_controller.h"
#include "UDPHandler.h"

#if HW_PLATFORM == 0
OTAUpdater_ESP12 updater;
#elif HW_PLATFORM == 1
OTAUpdater_ESP32 updater;
#endif

/* Network settings */
const char *ssid = "WLAN-4B5A8F"; // The SSID (name) of the Wi-Fi network you want to connect to
const char *password = "34620549741972173367";  // The password of the Wi-Fi network

/* MQTT settings */
const char* mqtt_server = "192.168.2.118";

/* Communication settings */
WiFiClient espClient;
PubSubClient client(espClient);
DynamicJsonBuffer jsonBuffer(50);

lamp_status current_status;
lamp_status status_request;

RGBcolor actualColor;

LEDController LED_controller(&status_request);

UDPHandler udp_handler(&status_request);

String IPAddress_string;
String MACAddress_string;
const char* ota_url;

enum system_state_var
{
  STARTUP = 0,
  NORMAL = 1,
  STREAMING = 2
};

system_state_var sysState;

init_struct initState;

uint8_t deviceID = 99;

void setup()
{
  
  Serial.begin(115200); // Start the Serial communication to send messages to the computer
  delay(10); 

  setup_wifi();
  delay(10);
  setup_mqtt();
  delay(100);
  //setup_OTA();
  //delay(100);
  setup_hardware();  

  sysState = STARTUP;

  /* Initial configuration of the lamp when the system is booted */
  status_request.lamp_mode = 1;
  status_request.color.R = 0;
  status_request.color.G = RGB_DEFAULT;
  status_request.color.B = 0;
  status_request.brightness = 1;
  status_request.effect_delay = 50;
  status_request.effect_speed = 50;
  status_request.streaming = false;

  /* Configuration state */
  initState.hasStarted = false;
  initState.isCompleted = false;

  /* Setup finished. Show leds */
  LED_controller.setLeds(status_request.color,0,NUM_LEDS/3);
}

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}

void setup_wifi()
{
  WiFi.begin(ssid, password); // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  WiFi.mode(WIFI_STA);
  
  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  MACAddress_string = WiFi.macAddress();

  /* Translate the IP address to String to have a unique name for MQTT client */
  IPAddress_string = IpAddress2String(WiFi.localIP());  
  
}

void setup_mqtt()
{
  /* Define MQTT broker */
  client.setServer(mqtt_server, 1883);
  /* Define callback function */
  client.setCallback(callback);
  /* Subscribe to topics */
  client.subscribe("lamp_network/mode_request");
  client.subscribe("lamp_network/light_intensity");
  client.subscribe("lamp_network/light_color");
  client.subscribe("lamp_network/effect_delay");
  client.subscribe("lamp_network/effect_speed");
  client.subscribe("lamp_network/alive_response");
  
}

void setup_hardware()
{  
  LED_controller.setup();  
}

void setup_OTA()
{
  updater.begin(ota_url);
}

/* Configure the callback function for a subscribed MQTT topic */
void callback(char* topic, byte* payload, unsigned int length) {

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
    status_request.lamp_mode = root["mode"];   
    Serial.println(status_request.lamp_mode);
  }
  
  else if(strcmp(topic,"lamp_network/light_intensity") == 0)
  {
    int rcv = root["intensity"];

    if(rcv == 0) rcv = 255;
    
    else
    {
      rcv = 11 - rcv;
    }
    
    status_request.brightness = rcv;
    Serial.println(rcv);
  }

  else if(strcmp(topic,"lamp_network/effect_delay") == 0)
  {
    int rcv = root["delay"];
    rcv = rcv * 10;
    status_request.effect_delay = rcv; //Delay in ms
    Serial.println(rcv);
  }

  else if(strcmp(topic,"lamp_network/effect_speed") == 0)
  {
    int rcv = root["speed"];
    rcv = 1000 - 10 * rcv;
    status_request.effect_speed = rcv; //Delay in ms
    Serial.println(rcv);
  }  

  else if(strcmp(topic,"lamp_network/light_color") == 0)
  {
    status_request.color.R = root["R"];
    status_request.color.G = root["G"];
    status_request.color.B = root["B"];

    // Output to serial monitor
#if 1
    Serial.println(status_request.color.R);
    Serial.println(status_request.color.G);
    Serial.println(status_request.color.B);
#endif
  }

  else if(strcmp(topic,"lamp_network/alive_response") == 0)
  {
    const char* mac_request = root["mac_origin"];

    if(strcmp(mac_request,MACAddress_string.c_str()) == 0)
    {
      deviceID = root["deviceID"];
      ota_url = root["OTA_URL"];    
    }
    
    initState.isCompleted = true;
  } 
}

/* Reconnect to the MQTT broker */
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(IPAddress_string.c_str())) //Unique name for each instance of a slave
    {
      Serial.println("connected");
      //Resubscribe
      client.subscribe("lamp_network/mode_request");
      client.subscribe("lamp_network/light_intensity");
      client.subscribe("lamp_network/light_color");
      client.subscribe("lamp_network/effect_delay");
      client.subscribe("lamp_network/effect_speed");
      client.subscribe("lamp_network/alive_response");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(500);
    }
  }  
}

void network_loop()
{
  /* MQTT loop */
  if (!client.connected()) reconnect();
  client.loop();

  //Serial.println("Network loop");

  //delay(1000);
  //client.publish("test_node/test", "");
}

void streaming_loop()
{
    
  //isStreaming = udp_handler.network_loop();
  

  if(!status_request.streaming)
  {
    udp_handler.stop();

    status_request.lamp_mode = 1;
    status_request.color.R = RGB_DEFAULT;
    status_request.color.G = RGB_DEFAULT;
    status_request.color.B = RGB_DEFAULT;

    sysState = NORMAL;
  }

  if(status_request.resync == true)
  {
    /* Do effect resynchronization */
    LED_controller.resync();
    status_request.resync = false;
  }  
}

void status_update()
{ 
  
  /* Check difference in mode request */
  if(status_request.lamp_mode != current_status.lamp_mode)
  {
    /* Finish previous effect */
    LED_controller.end_effect();
    
    /* Streaming request */
    if(status_request.lamp_mode == 3)
    {
      Serial.println("Streaming request received");
      /* Start UDP socket */
      udp_handler.begin();
      status_request.streaming = true;
      sysState = STREAMING;
      /* Go to lamp mode 2 to show a demo effect */
      status_request.lamp_mode = 2;
    }

    /* Streaming request */
    if(status_request.lamp_mode == 1)
    {
      Serial.println("ON request received");
      status_request.color.R = RGB_DEFAULT;
      status_request.color.G = RGB_DEFAULT;
      status_request.color.B = RGB_DEFAULT;
      status_request.effect_delay = 50;
      status_request.effect_speed = 50;
      status_request.streaming = false;
    }
    
    Serial.print("Received change request to mode ");
    Serial.println(status_request.lamp_mode);
       
    current_status.lamp_mode = status_request.lamp_mode; 
    LED_controller.update_mode();   
  }

  if(status_request.brightness != current_status.brightness)
  {
    Serial.print("Received change request to brightness level ");
    Serial.println(status_request.brightness);    

    current_status.brightness = status_request.brightness;  
    LED_controller.update_mode();
  }

  if(status_request.color.R != current_status.color.R || status_request.color.G != current_status.color.G || status_request.color.B != current_status.color.B)
  {
    Serial.print("Received change request to color: ");
    Serial.println(status_request.color.R);
    Serial.println(status_request.color.G);   
    Serial.println(status_request.color.B);    

    current_status.brightness = status_request.brightness;  

    LED_controller.update_color();

    current_status.color.R = status_request.color.R;
    current_status.color.G = status_request.color.G;
    current_status.color.B = status_request.color.B;     
  }
}

void initComm()
{
  /* Send a MQTT request */
  if(!initState.hasStarted)
  {
    LED_controller.setLeds(status_request.color,0,(NUM_LEDS*2)/3);
    client.publish("lamp_network/alive_request", MACAddress_string.c_str());
    initState.hasStarted = true;
    initState.elapsed_time = millis();
  }

  /* Wait asynchronously for the answer */
  if(initState.hasStarted)
  {
    /* Check if answer was received */
    if(initState.isCompleted)
    {
      sysState = NORMAL;
      setup_OTA();
      
      LED_controller.setAllLeds(status_request.color,0);
      delay(1000);
      status_request.color.R = RGB_DEFAULT;
      status_request.color.G = RGB_DEFAULT;
      status_request.color.B = RGB_DEFAULT;
      
      Serial.print("Successfull communication setup. Device ID: ");
      Serial.println(deviceID);

      
      
      return;
    }
    /* Timeout. Show error and reset */
    else if( (millis() - initState.elapsed_time) > initState.timeout )
    {
      status_request.color.R = RGB_DEFAULT;
      status_request.color.G = 0;
      status_request.color.B = 0;
      LED_controller.setAllLeds(status_request.color,0);

      Serial.println("Error in communication setup. Restarting ESP32");

      delay(1000);

      ESP.restart();
    }
  }
}

void loop()
{

  switch(sysState)
  {
    case STARTUP:
      network_loop();
      initComm();
      break;
      
    case NORMAL:
      network_loop();
      updater.OTA_handle();
      status_update();
      LED_controller.feed();   
      break;
      
    case STREAMING:
      streaming_loop();  
      LED_controller.feed(); 
  }
    
}
