#if !defined COMON_DATATYPES_H
#define COMON_DATATYPES_H

#include "config.h"

/* Logical states */
template <typename T>
struct state_tracker
{
  T val;
  T old;
};

/* ------- ENUMS -------- */
/* Status of the system */
enum system_state_var
{
  STARTUP = 0, // Initial communication handshake (MQTT)
  NORMAL = 1 // Normal mode
};

/* Communication type */
enum COMM_TYPE
{
  MQTT = 0,
  UDP = 1
};

/* Music effects */
enum MusicMode
{
  BUBBLE = 0,
  ENERGY_BAR = 1,
  ENERGY_BAR_COLOR = 2,
  SPECTRUM_BAR = 3,
  SPECTRUM_BAR_COLOR = 4,
  FULL_SPECTRUM_COLOR = 5
};

/* Static effects */
enum StaticMode
{
  RGB_LOOP = 0,
  FADE_IN_OUT = 1,
  STROBE = 2,
  FIRE = 3,
  HALLOWEEN_EYES = 4,
  CYCLON_BOUNCE = 5,
  TWINKLE = 6,
  TWINKLE_RANDOM = 7,
  SPARKLE = 8,
  SNOW_SPARKLE = 9,
  RUNNING_LIGHTS = 10,
  COLOR_WIPE = 11,
  RAINBOW_CYCLE = 12,
  THEATER_CHASE = 13,
  THEATER_CHASE_RAINBOW = 14,
  BOUNCING_COLORED_BALLS = 15,
  METEOR_RAIN = 16,
  FADE_TO_BLACK = 17
};

/* UDP message identifiers */
enum UDP_Message_Id
{
  MODE_SELECT = 0,
  SYNC_REQ = 1,  
  ACK = 2,
  ERR = 3,
  PAYLOAD_SINGLE = 4,
  PAYLOAD_WINDOW = 5,
  PAYLOAD_FULL = 6
};

/* ------- STRUCTS ------- */
/* Initial communication handshake */
struct init_struct
{
  bool hasStarted; //The handshake has started
  bool isCompleted; //The handshake is completed
  const unsigned long timeout = INIT_COMM_TIMEOUT; //Handshake timeout
  unsigned long elapsed_time; //Handshake timer
};

/* Color info encapsulation */
struct RGBcolor
{
  uint8_t R;
  uint8_t G;
  uint8_t B;
};

/* Lamp main shared memory */
struct lamp_status
{
  uint8_t lamp_mode; //Current lamp mode
  uint8_t amplitude; //Effect amplitude
  uint8_t brightness; //Effect brightness
  uint8_t deviceID; //Lamp ID
  RGBcolor color; //Lamp color
  uint32_t effect_delay; //Effect delay
  uint32_t effect_speed; //Effect speed
  uint32_t effect_amount; //Generic configuration parameter available for different effects. Meaning may vary.
  float light_amount; //Current light amount
  String IPAddress_string; //IP address
  String MACAddress_string; //MAC address
  String ota_url; //URL for OTA sofware updates
  bool resync; //Resynchronization request pending  
  system_state_var sysState; //System state
  init_struct initState; //Initialization state
};

/* -------- UDP interface description --------- */

/* Generic UDP payload */
struct udp_payload 
{
  uint8_t mask; //To address a payload to specific slaves
  RGBcolor color;  //Color information
  uint8_t amplitude; //Amplitude information
};

/* UDP message structure with single payload */
struct udp_payload_msg
{
  uint8_t msgID;
  udp_payload payload;
};

/* UDP message structure with 6 payload elements */
/* Used to receive color information from up to 6 different spectrum windows */
struct udp_payload_window_spectrum_msg
{
  uint8_t msgID;
  udp_payload payload[6]; //6 is the maximum allowed nubmer of lamps
};

/* UDP message structure with a payload element for each number of LEDs */
struct udp_payload_full_spectrum_msg
{
  uint8_t msgID;
  RGBcolor color[NUM_LEDS]; //One RGB value for each LED
};

/* UDP message to set a mode */
struct udp_mode_select 
{
  uint8_t msgID;
  uint8_t mode_select;
  uint8_t mode_properties; //TODO: expand this when all properties are known
};

/* UDP synchronization request */
struct udp_sync_req
{
  uint8_t msgID;
  uint8_t delay_ms; //Option to add a delay to the synchronization (currently not supported)
};

/* UDP acknowledgement. Currently not supported */
struct udp_ack 
{
  uint8_t msgID;
  uint8_t nodeID;
  uint8_t ackID;
};


#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
