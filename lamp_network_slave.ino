
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

bool isStreaming = false;

lamp_status current_status;
lamp_status status_request;

RGBcolor actualColor;

LEDController LED_controller(&status_request);

UDPHandler udp_handler(&status_request);

String IPAddress_string;

void setup()
{
  
  Serial.begin(115200); // Start the Serial communication to send messages to the computer
  delay(10); 

  setup_wifi();
  setup_OTA();
  setup_mqtt();
  setup_hardware();  

  /* Initial configuration of the lamp when the system is booted */
  status_request.lamp_mode = 1;
  status_request.color.R = 20;
  status_request.color.G = 20;
  status_request.color.B = 20;
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
}

void setup_hardware()
{  
  LED_controller.setup();  
}

void setup_OTA()
{
  updater.begin();
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
    status_request.brightness = rcv;
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
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }  
}

void network_loop()
{
  /* MQTT loop */
  if (!client.connected()) reconnect();
  client.loop();
}

void streaming_loop()
{
    
  isStreaming = udp_handler.network_loop();

  if(!isStreaming)
  {
    udp_handler.stop();

    status_request.lamp_mode = 1;
    status_request.color.R = 20;
    status_request.color.G = 20;
    status_request.color.B = 20;
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
    /* Streaming request */
    if(status_request.lamp_mode == 3)
    {
      Serial.println("Streaming request received");
      /* Start UDP socket */
      udp_handler.begin();
      isStreaming = true;
      /* Go to lamp mode 2 to show a demo effect */
      status_request.lamp_mode = 2;
    }    
    
    Serial.print("Received change request to mode ");
    Serial.println(status_request.lamp_mode);
       
    LED_controller.update_mode();   
    current_status.lamp_mode = status_request.lamp_mode;   
  }

  if(status_request.brightness != current_status.brightness)
  {
    Serial.print("Received change request to brightness level ");
    Serial.println(status_request.brightness);    

    current_status.brightness = status_request.brightness;  
    LED_controller.update_brightness();
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

void loop()
{

  /* UDP streaming. Handle only UDP communication */
  if(isStreaming)
  {
    streaming_loop();    
  }

  /* No streaming. Handle MQTT and OTA */
  else
  {
    network_loop();
    updater.OTA_handle();
    status_update();
  }

  /* Feed the LED controller with the latest available info */
  LED_controller.feed();   
    
}
