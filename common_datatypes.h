#if !defined COMON_DATATYPES_H
#define COMON_DATATYPES_H


struct sync_request {
  uint8_t msgID;
  uint8_t msgContent;
};

struct color_request {
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
};

enum StaticMode
{
  RGB_LOOP = 0,
  FADE_IN_OUT = 1,
  STROBE = 2  
};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
