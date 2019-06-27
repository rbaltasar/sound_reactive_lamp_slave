
#include <rom/rtc.h>
#include "timeSync.h"
# include <WiFi.h>
#include "LED_controller.h"
#include "UDPHandler.h"
#include "OTA_updater_ESP32.h"
#include "MQTTHandler.h"
#include "UDPHandler.h"
#include "network_credentials.h"

/* ------------------------ */
/* --- Global variables --- */
/* ------------------------ */

/* Web Server for OTA softare updates */
OTAUpdater_ESP32 updater;
/* Shared memory containing the global configuration and state of the lamp */
/* This object is shared within the LED Controller and the Communication Handler */
state_tracker<lamp_status> lamp_state;
/* Shared synchronized timer */
timeSync timer;
/* LED controller object */
LEDController LED_controller(&lamp_state.val,&timer);
/* Polymorphic pointer. Shall point to the currently used communication class */
CommunicationHandler* communication_handler;
/* MQTT communication handler */
MQTTHandler mqtt_handler(&lamp_state.val,&timer);
/* UDP communication handler */
UDPHandler udp_handler(&lamp_state.val,&timer);


/* Reseat reason. Only for debugging reasons. Remove long term */
RESET_REASON reset_reason_0;
RESET_REASON reset_reason_1;

void setup()
{
  
  Serial.begin(115200);
  delay(10);

  /* Get reset reason */
  get_reset_reason();

  /* Setup the WiFi connection */
  setup_wifi();
  /* Start with MQTT communication. Create object */  
  communication_handler = &mqtt_handler;
  /* Setup the hardware */
  setup_hardware(); 

  /* Initial configuration of the lamp when the system is booted */
  lamp_state.val.lamp_mode = 99;
  lamp_state.val.color.R = 0;
  lamp_state.val.color.G = G_DEFAULT;
  lamp_state.val.color.B = 0;
  lamp_state.val.brightness = 1;
  lamp_state.val.effect_delay = 50;
  lamp_state.val.effect_speed = 50; 
  lamp_state.val.effect_amount = 1;
  lamp_state.val.sysState = STARTUP;

  /* Same configuration for the "old" variable. TODO: create copy operator */
  lamp_state.old.lamp_mode = 99;
  lamp_state.old.color.R = 0;
  lamp_state.old.color.G = G_DEFAULT;
  lamp_state.old.color.B = 0;
  lamp_state.old.brightness = 1;
  lamp_state.old.effect_delay = 50;
  lamp_state.old.effect_speed = 50; 
  lamp_state.old.effect_amount = 1;

  /* Init state */
  lamp_state.val.initState.hasStarted = false;
  lamp_state.val.initState.isCompleted = false;

  /* Setup finished. Show leds */
  //LED_controller.setLeds(lamp_state.val.color,0,NUM_LEDS/3);
}

/* Get the IP address in String format */
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}

/* Print reset reason. TODO: remove long term */
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

/* Setup WiFi connection */
void setup_wifi()
{
  WiFi.begin(ssid, password); // Connect to the network. Use network_credentials.h
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

  /* Get MAC address */
  lamp_state.val.MACAddress_string = WiFi.macAddress();

  /* Translate the IP address to String to have a unique name for MQTT client */
  lamp_state.val.IPAddress_string = IpAddress2String(WiFi.localIP());  
}

/* Setup the LED controller */
void setup_hardware()
{  
  LED_controller.setup();
  pinMode(LED_BUILTIN, OUTPUT);
}

/* Setup the OTA software update web server */
void setup_OTA()
{
  updater.begin(lamp_state.val.ota_url.c_str());
}

/* Update the current lamp mode */
/* Switch between communication handlers when necessary */
void mode_update()
{
  Serial.print("Received change request to mode ");
  Serial.println(lamp_state.val.lamp_mode);

  /* Finish previous effect */
  LED_controller.end_effect();
  
  /* Switch communication handlers if necessary */
  if(lamp_state.val.lamp_mode >= 100 && lamp_state.old.lamp_mode < 100) //Switch from MQTT to UDP
  {
    /* Stop MQTT communication handler */
    communication_handler->stop();
    /* Assign UDP communication handler */
    communication_handler = &udp_handler;    
    /* Start UDP communication handler */
    communication_handler->begin();
  }
  else if(lamp_state.old.lamp_mode >= 100 && lamp_state.val.lamp_mode < 100) //Switch from UDP to MQTT
  {
    /* Stop UDP communication handler */
    communication_handler->stop();
    /* Assign MQTT communication handler */
    communication_handler = &mqtt_handler;
    /* Start MQTT communication handler */
    communication_handler->begin();
  } 

  /* ON request - force default colors */
  if(lamp_state.val.lamp_mode == 1)
  {
    Serial.println("ON request received");
    lamp_state.val.color.R = R_DEFAULT;
    lamp_state.val.color.G = G_DEFAULT;
    lamp_state.val.color.B = B_DEFAULT;     
  }
  
    
  /* Shadow current mode for evaluation of new requests */
  lamp_state.old.lamp_mode = lamp_state.val.lamp_mode; 
  
  /* Inform the LED controller about the update */
  LED_controller.update_mode();   
}

/* Check for update requests */
void status_update()
{   
  /* Check difference in mode request */
  if(lamp_state.val.lamp_mode != lamp_state.old.lamp_mode)
  {
    mode_update();
  }

  /* Check difference in light */
  if((lamp_state.val.light_amount != lamp_state.old.light_amount) && (lamp_state.val.lamp_mode == 4))
  {
    LED_controller.update_mode();
    lamp_state.old.light_amount = lamp_state.val.light_amount;
  }

  /* Check difference in brightness request */
  else if(lamp_state.val.brightness != lamp_state.old.brightness)
  {
    Serial.print("Received change request to brightness level ");
    Serial.println(lamp_state.val.brightness);    

    /* Shadow current change for evaluation of new requests */
    lamp_state.old.brightness = lamp_state.val.brightness;  
    
    /* Inform the LED controller about the update */
    LED_controller.update_mode();
  }

  /* Check difference in color request */
  else if(lamp_state.val.color.R != lamp_state.old.color.R || lamp_state.val.color.G != lamp_state.old.color.G || lamp_state.val.color.B != lamp_state.old.color.B)
  {
    Serial.print("Received change request to color: ");
    Serial.println(lamp_state.val.color.R);
    Serial.println(lamp_state.val.color.G);   
    Serial.println(lamp_state.val.color.B);  

    /* Shadow current change for evaluation of new requests */
    lamp_state.old.color.R = lamp_state.val.color.R;
    lamp_state.old.color.G = lamp_state.val.color.G;
    lamp_state.old.color.B = lamp_state.val.color.B;
    
    /* Inform the LED controller about the update */
    LED_controller.update_mode();
  }

  /* Check resync request */
  if(lamp_state.val.resync)
  {
    /* Resynchronize led controller */
    LED_controller.resync();

    /* Deactivate resync flag */
    lamp_state.val.resync = false;
  }
}

/* Handle initial communication handshake to ensurue stable MQTT communication with the broker */
void initComm()
{
  /* Send a MQTT request */
  if(!lamp_state.val.initState.hasStarted)
  {
    /* Give visual feedback */
    //LED_controller.setLeds(lamp_state.val.color,0,(NUM_LEDS*2)/3);

    /* Initiate handshake. TODO: check that MQTT communication is being used */
    communication_handler->publish_initcomm();
    
    /* Update handshake state */
    lamp_state.val.initState.hasStarted = true;
    lamp_state.val.initState.elapsed_time = millis();
  }

  /* Wait asynchronously for the answer */
  else if(lamp_state.val.initState.hasStarted)
  {
    /* Check if answer was received */
    if(lamp_state.val.initState.isCompleted)
    {
      /* Finish the handshake */
      communication_handler->finish_initcomm();
      
      /* Update system state. Go to normal mode */
      lamp_state.val.sysState = NORMAL;
      
      /* Begin OTA */
      setup_OTA();

      delay(500);
      
      /* Give visual feedback */
      LED_controller.setAllLeds(lamp_state.val.color,0);      
      
      Serial.print("Successfull communication setup. Device ID: ");
      Serial.println(lamp_state.val.deviceID);

      return;
    }
    /* Timeout. Show error and reset */
    else if( (millis() - lamp_state.val.initState.elapsed_time) > lamp_state.val.initState.timeout )
    {
      Serial.println("Error in communication setup. Restarting ESP32");
      
      /* Give visual feedback */
      lamp_state.val.color.R = R_DEFAULT;
      lamp_state.val.color.G = 0;
      lamp_state.val.color.B = 0;
      LED_controller.setAllLeds(lamp_state.val.color,0);
      delay(1000);

      /* Restart microcontroller */
      ESP.restart();
    }
  }
}

/* Main loop */
void loop()
{
  /* Execute the network loop of the currently used communication handler */
  communication_handler->network_loop();

  switch(lamp_state.val.sysState)
  {
    case STARTUP:
      /* Handle initial communication handshake */
      initComm();
      break;
      
    case NORMAL: 
      /* Handle OTA */
      if(communication_handler->get_comm_type() == MQTT) updater.OTA_handle();
      /* Check for requested updates */
      status_update();
      /* Feed the LED controller */
      LED_controller.feed();   
      break;
  }
}
