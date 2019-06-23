
#if !defined LEDMUSICCEFFECTS_H
#define LEDMUSICCEFFECTS_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include "config.h"
#include <FastLED.h>
#include "LED_controller.h"
#include "timeSync.h"

class LEDController;

class LEDMusicEffects
{

private:

  LEDController* m_led_controller;
  CRGB* m_leds;
  timeSync* m_timer;
  
  unsigned long m_last_iteration;

  void shift_leds(uint8_t positions, const bool right, const uint8_t delay_ms, const uint8_t R_in, const uint8_t G_in, const uint8_t B_in);

public:

  LEDMusicEffects(LEDController* led_controller, CRGB* leds_ptr, timeSync* timer);
  ~LEDMusicEffects();

  void end_effect();
  void resync();

  
  void streaming_1(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude);
};

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
