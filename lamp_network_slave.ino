
#include <rom/rtc.h>
#include "timeSync.h"
# include <WiFi.h>
#include "LED_controller.h"
#include "UDPHandler.h"
#include "OTA_updater_ESP32.h"
#include "MQTTHandler.h"
#include "network_credentials.h"

OTAUpdater_ESP32 updater;

state_tracker<lamp_status> lamp_state;

LEDController LED_controller(&lamp_state.val);

UDPHandler udp_handler(&lamp_state.val);

RESET_REASON reset_reason_0;
RESET_REASON reset_reason_1;

CommunicationHandler* communication_handler;

void setup()
{
  
  Serial.begin(115200); // Start the Serial communication to send messages to the computer
  delay(10);

  /* Get reset reason */
  get_reset_reason();

  setup_wifi();
  delay(100);
  communication_handler = new MQTTHandler(&lamp_state.val);
  delay(200);  
  setup_hardware();  

  lamp_state.val.sysState = STARTUP;

  /* Initial configuration of the lamp when the system is booted */
  lamp_state.val.color.R = 0;
  lamp_state.val.color.G = G_DEFAULT;
  lamp_state.val.color.B = 0;
  lamp_state.val.brightness = 1;
  lamp_state.val.effect_delay = 50;
  lamp_state.val.effect_speed = 50; 
  lamp_state.val.effect_amount = 1;

  lamp_state.old.color.R = 0;
  lamp_state.old.color.G = G_DEFAULT;
  lamp_state.old.color.B = 0;
  lamp_state.old.brightness = 1;
  lamp_state.old.effect_delay = 50;
  lamp_state.old.effect_speed = 50; 
  lamp_state.old.effect_amount = 1;

  /* Configuration state */
  lamp_state.val.initState.hasStarted = false;
  lamp_state.val.initState.isCompleted = false;

  /* Setup finished. Show leds */
  LED_controller.setLeds(lamp_state.val.color,0,NUM_LEDS/3);
}

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}

void print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1  : Serial.println ("Vbat power on reset");break;
    case 3  : Serial.println ("Software reset digital core");break;
    case 4  : Serial.println ("Legacy watch dog reset digital core");break;
    case 5  : Serial.println ("Deep Sleep reset digital core");break;
    case 6  : Serial.println ("Reset by SLC module, reset digital core");break;
    case 7  : Serial.println ("Timer Group0 Watch dog reset digital core");break;
    case 8  : Serial.println ("Timer Group1 Watch dog reset digital core");break;
    case 9  : Serial.println ("RTC Watch dog Reset digital core");break;
    case 10 : Serial.println ("Instrusion tested to reset CPU");break;
    case 11 : Serial.println ("Time Group reset CPU");break;
    case 12 : Serial.println ("Software reset CPU");break;
    case 13 : Serial.println ("RTC Watch dog Reset CPU");break;
    case 14 : Serial.println ("for APP CPU, reseted by PRO CPU");break;
    case 15 : Serial.println ("Reset when the vdd voltage is not stable");break;
    case 16 : Serial.println ("RTC Watch dog reset digital core and rtc module");break;
    default : Serial.println ("NO_MEAN");
  }
}

void get_reset_reason()
{

  reset_reason_0 = rtc_get_reset_reason(0);
  reset_reason_1 = rtc_get_reset_reason(1);
  
  Serial.println("CPU0 reset reason:");
  print_reset_reason(reset_reason_0);

  Serial.println("CPU1 reset reason:");
  print_reset_reason(reset_reason_0);
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

  lamp_state.val.MACAddress_string = WiFi.macAddress();

  /* Translate the IP address to String to have a unique name for MQTT client */
  lamp_state.val.IPAddress_string = IpAddress2String(WiFi.localIP());  
  
}

void setup_hardware()
{  
  LED_controller.setup();
  pinMode(LED_BUILTIN, OUTPUT);
}

void setup_OTA()
{
  updater.begin(lamp_state.val.ota_url);
}


#if 0
void streaming_loop()
{
    
  //isStreaming = udp_handler.network_loop();
  

  if(!status_request.streaming)
  {
    udp_handler.stop();

    status_request.lamp_mode = 1;
    status_request.color.R = R_DEFAULT;
    status_request.color.G = G_DEFAULT;
    status_request.color.B = B_DEFAULT;

    sysState = NORMAL;
  }

  if(status_request.resync == true)
  {
    /* Do effect resynchronization */
    LED_controller.resync();
    status_request.resync = false;
  }  
}
#endif

void mode_update()
{
  /* Finish previous effect */
  LED_controller.end_effect();
  
  /* Streaming request */
  if(lamp_state.val.lamp_mode == 3)
  {
    Serial.println("Streaming request received");
    /* Start UDP socket */
    udp_handler.begin();
  
    lamp_state.val.sysState = STREAMING;
    /* Go to lamp mode 2 to show a demo effect */
    lamp_state.val.lamp_mode = 2;
  }

  /* Streaming request */
  if(lamp_state.val.lamp_mode == 1)
  {
    Serial.println("ON request received");
    lamp_state.val.color.R = R_DEFAULT;
    lamp_state.val.color.G = G_DEFAULT;
    lamp_state.val.color.B = B_DEFAULT;
    lamp_state.val.effect_delay = 50;
    lamp_state.val.effect_speed = 50;   
  }
  
  Serial.print("Received change request to mode ");
  Serial.println(lamp_state.val.lamp_mode);
     
  lamp_state.old.lamp_mode = lamp_state.val.lamp_mode; 
  LED_controller.update_mode();   
}

void status_update()
{ 
  
  /* Check difference in mode request */
  if(lamp_state.val.lamp_mode != lamp_state.old.lamp_mode)
  {
    mode_update();
  }

  else if(lamp_state.val.brightness != lamp_state.old.brightness)
  {
    Serial.print("Received change request to brightness level ");
    Serial.println(lamp_state.val.brightness);    

    lamp_state.old.brightness = lamp_state.val.brightness;  
    LED_controller.update_mode();
  }

  else if(lamp_state.val.color.R != lamp_state.old.color.R || lamp_state.val.color.G != lamp_state.old.color.G || lamp_state.val.color.B != lamp_state.old.color.B)
  {
    Serial.print("Received change request to color: ");
    Serial.println(lamp_state.val.color.R);
    Serial.println(lamp_state.val.color.G);   
    Serial.println(lamp_state.val.color.B);    

    lamp_state.val.brightness = lamp_state.val.brightness;  

    LED_controller.update_mode();

    lamp_state.old.color.R = lamp_state.val.color.R;
    lamp_state.old.color.G = lamp_state.val.color.G;
    lamp_state.old.color.B = lamp_state.val.color.B;     
  }
}

void initComm()
{
  /* Send a MQTT request */
  if(!lamp_state.val.initState.hasStarted)
  {
    LED_controller.setLeds(lamp_state.val.color,0,(NUM_LEDS*2)/3);

    communication_handler->publish_initcomm();
    
    lamp_state.val.initState.hasStarted = true;
    lamp_state.val.initState.elapsed_time = millis();
  }

  /* Wait asynchronously for the answer */
  else if(lamp_state.val.initState.hasStarted)
  {
    /* Check if answer was received */
    if(lamp_state.val.initState.isCompleted)
    {

      communication_handler->finish_initcomm();
      
      lamp_state.val.sysState = NORMAL;
      setup_OTA();

      delay(1000);
      
      LED_controller.setAllLeds(lamp_state.val.color,0);
      
      lamp_state.val.color.R = R_DEFAULT;
      lamp_state.val.color.G = G_DEFAULT;
      lamp_state.val.color.B = B_DEFAULT;
      
      Serial.print("Successfull communication setup. Device ID: ");
      Serial.println(lamp_state.val.deviceID);

      return;
    }
    /* Timeout. Show error and reset */
    else if( (millis() - lamp_state.val.initState.elapsed_time) > lamp_state.val.initState.timeout )
    {
      lamp_state.val.color.R = R_DEFAULT;
      lamp_state.val.color.G = 0;
      lamp_state.val.color.B = 0;
      LED_controller.setAllLeds(lamp_state.val.color,0);

      Serial.println("Error in communication setup. Restarting ESP32");

      delay(1000);

      ESP.restart();
    }
  }
}

void loop()
{
  communication_handler->network_loop();

  switch(lamp_state.val.sysState)
  {
    case STARTUP:      
      initComm();
      break;
      
    case NORMAL:     
      updater.OTA_handle();
      status_update();
      LED_controller.feed();   
      break;
  }
}
