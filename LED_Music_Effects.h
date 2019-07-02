
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
  uint8_t music_effect_mem[4];
  CRGB m_static_color[NUM_LEDS];
  
  unsigned long m_last_iteration;

  void shift_leds(uint8_t led_start, uint8_t led_end, uint8_t positions, const bool top, const uint8_t delay_ms, const uint8_t R_in, const uint8_t G_in, const uint8_t B_in);
  void print_amplitude_color(uint8_t led_start, uint8_t led_end, const bool top, uint8_t amplitude, uint8_t r, uint8_t g, uint8_t b);
  void print_amplitude_static(uint8_t led_start, uint8_t led_end, const bool top, uint8_t amplitude, uint8_t r_base, uint8_t g_base, uint8_t b_base);
  void generate_static_colors(uint8_t r_base, uint8_t g_base, uint8_t b_base);
  bool is_update(uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude);

  
public:

  LEDMusicEffects(LEDController* led_controller, CRGB* leds_ptr, timeSync* timer);
  ~LEDMusicEffects();

  void end_effect();
  void resync();

  
  void bubble_effect(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude, uint8_t direction);
  void power_bars_effect(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude,uint8_t direction, uint8_t effect_type);
};

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
