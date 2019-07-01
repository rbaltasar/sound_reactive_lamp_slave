
#include "LED_Music_Effects.h"


LEDMusicEffects::LEDMusicEffects(LEDController* led_controller, CRGB* leds_ptr, timeSync* timer):
m_led_controller(led_controller),
m_leds(leds_ptr),
m_timer(timer)
{
  //start_sequence = true;
  m_last_iteration = 0;
}

LEDMusicEffects::~LEDMusicEffects()
{
  //end_effect();
}

void LEDMusicEffects::end_effect()
{
  //start_sequence = true;
  
}

void LEDMusicEffects::resync()
{
  m_last_iteration = m_timer->getTime();
}

void LEDMusicEffects::shift_leds(uint8_t led_start, uint8_t led_end, uint8_t positions, const bool top, const uint8_t delay_ms, const uint8_t R_in, const uint8_t G_in, const uint8_t B_in)
{
  if(led_end <= led_start) return;
  
  if(positions == 0) positions = 1;
  if( (led_end - led_start) < positions ) positions = 1;
  
  if(top)
  {
    for(uint8_t j = led_end - 1; j >= positions ; j--)
    {
      m_leds[j] = m_leds[j-positions];
      //Serial.println(j);
      //delay(100);
    }
    for(uint8_t j = led_start; j < positions; j++)
    {
      m_leds[j] = CRGB(R_in,G_in,B_in);
      //Serial.println(j);
      //delay(100);
    }
  }

  else
  {
    for(uint8_t j = led_start; j < (led_end - positions); j++)
    {
      m_leds[j] = m_leds[j+positions];
    }
    for(uint8_t j = led_end - 1; j < led_end - positions; j--)
    {
      m_leds[j] = CRGB(R_in,G_in,B_in);
    }
  }

  FastLED.show();
  delay(delay_ms);
}

void LEDMusicEffects::bubble_effect(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude, DirectionType direction)
{
  unsigned long now = m_timer->getTime();

  if( (now - m_last_iteration) > print_delay )
  {
    m_last_iteration = now;
    
    if(direction == UP)
    {
      shift_leds(0, NUM_LEDS, amplitude, true, 0, r, g, b);
    }
    else if(direction == DOWN)
    {
      shift_leds(0, NUM_LEDS, amplitude, true, 0, r, g, b);
    }
    if(direction == MIDDLE)
    {
      shift_leds(NUM_LEDS / 2, NUM_LEDS, amplitude, true, 0, r, g, b);
      shift_leds(0, NUM_LEDS / 2 , amplitude, false, 0, r, g, b);
    }
  }
}

void LEDMusicEffects::power_bars_effect(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude)
{
  unsigned long now = m_timer->getTime();

  if( (now - m_last_iteration) > print_delay )
  {
    m_last_iteration = now;

    for(uint8_t i = 0; i < amplitude; i++)
    {
      m_leds[i] = CRGB(r,g,b);
    }
    for(uint8_t i = amplitude; i < NUM_LEDS; i++)
    {
      m_leds[i] = CRGB(0,0,0);
    }
    FastLED.show();
  }
}


/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
