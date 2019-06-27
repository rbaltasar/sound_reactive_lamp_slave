
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

void LEDMusicEffects::shift_leds(uint8_t positions, const bool right, const uint8_t delay_ms, const uint8_t R_in, const uint8_t G_in, const uint8_t B_in)
{
  if(positions == 0) positions = 1;
  
  if(right)
  {
    for(uint8_t j = NUM_LEDS - 1; j >= positions ; j--)
    {
      m_leds[j] = m_leds[j-positions];
      //Serial.println(j);
      //delay(100);
    }
    for(uint8_t j = 0; j < positions; j++)
    {
      m_leds[j] = CRGB(R_in,G_in,B_in);
      //Serial.println(j);
      //delay(100);
    }
  }

  else
  {
    for(uint8_t j = 0; j < (NUM_LEDS - positions); j++)
    {
      m_leds[j] = m_leds[j+positions];
    }
    for(uint8_t j = NUM_LEDS - 1; j < NUM_LEDS - positions; j--)
    {
      m_leds[j] = CRGB(R_in,G_in,B_in);
    }
  }

  FastLED.show();
  delay(delay_ms);
}

void LEDMusicEffects::streaming_1(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude)
{
  unsigned long now = m_timer->getTime();

  if( (now - m_last_iteration) > print_delay )
  {
    m_last_iteration = now;
    
    shift_leds(amplitude, true, 0, r, g, b);
  }
}

void LEDMusicEffects::power_bars(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude)
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
