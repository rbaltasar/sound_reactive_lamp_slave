#if !defined COMON_DATATYPES_H
#define COMON_DATATYPES_H


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

struct color_request 
{
  uint8_t msgID;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
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
  uint8_t brightness;
  bool resync;
  bool streaming;
  uint32_t effect_delay;
  uint32_t effect_speed;
};

enum StaticMode
{
  RGB_LOOP = 0,
  FADE_IN_OUT = 1,
  STROBE = 2,
  FIRE = 3,
  HALLOWEEN_EYES = 4,
  CYCLON_BOUNCE = 5,
  NEW_KITT = 6
};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
