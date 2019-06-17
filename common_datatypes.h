#if !defined COMON_DATATYPES_H
#define COMON_DATATYPES_H

/* Logical states */
template <typename T>
struct state_tracker
{
  T val;
  T old;
};

enum system_state_var
{
  STARTUP = 0,
  NORMAL = 1,
  STREAMING = 2
};

enum COMM_TYPE
{
  MQTT = 0,
  UDP = 1
};

struct sync_request 
{
  uint8_t msgID;
  uint8_t msgContent;
};

struct init_struct
{
  bool hasStarted;
  bool isCompleted;
  const unsigned long timeout = INIT_COMM_TIMEOUT;
  unsigned long elapsed_time;
};

struct RGBcolor
{
  uint8_t R;
  uint8_t G;
  uint8_t B;
};

struct lamp_status
{
  uint8_t lamp_mode;
  RGBcolor color;
  uint8_t amplitude;
  uint8_t brightness;
  uint8_t deviceID;
  uint32_t effect_delay;
  uint32_t effect_speed;
  uint32_t effect_amount;
  float light_amount;
  String IPAddress_string;
  String MACAddress_string;
  const char* ota_url;
  system_state_var sysState;
  init_struct initState;
};

enum MusicMode
{
  STREAMING_1 = 0,
  STREAMING_2 = 1
};

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

/* UDP interface description */

enum UDP_Message_Id
{
  MODE_SELECT = 0,
  SYNC_REQ = 1,
  PAYLOAD = 2,
  ACK = 3,
  ERR = 4
};

struct udp_payload 
{
  uint8_t msgID;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t amplitude;
};

struct udp_mode_select 
{
  uint8_t msgID;
  uint8_t mode_select;
};

struct udp_sync_req
{
  uint8_t msgID;
  uint8_t delay_ms;
};

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
