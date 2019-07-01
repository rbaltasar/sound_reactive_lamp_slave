
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

void LEDMusicEffects::print_amplitude_color(uint8_t led_start, uint8_t led_end, const bool top, uint8_t amplitude, uint8_t r, uint8_t g, uint8_t b)
{
  if(top)
  {    
    for(uint8_t i = led_start; i < amplitude; i++)
    {
      m_leds[i] = CRGB(r,g,b);
    }
    for(uint8_t i = amplitude; i < led_end; i++)
    {
      m_leds[i] = CRGB(0,0,0);
    }
  }
  else
  {
    for(uint8_t i = led_start; i < (led_end - amplitude); i++)
    {
      m_leds[i] = CRGB(0,0,0);
    }
    for(uint8_t i = (led_end - amplitude); i < led_end; i++)
    {
      m_leds[i] = CRGB(r,g,b);
    }    
  }
   FastLED.show();  
}


void LEDMusicEffects::generate_static_colors(uint8_t r_base, uint8_t g_base, uint8_t b_base)
{

  /* Do something with m_static_color */
}

bool LEDMusicEffects::is_update(uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude)
{
  bool retVal = false;
  
  uint8_t r_old = music_effect_mem[0];
  uint8_t g_old = music_effect_mem[1];
  uint8_t b_old = music_effect_mem[2];
  uint8_t ampl_old = music_effect_mem[3];
  
  if( (r_old != r) || (g_old != g) || (b_old != b) )
  {
    /* Update effect memory */
    music_effect_mem[0] = r;
    music_effect_mem[1] = g;
    music_effect_mem[2] = b;
    
    retVal = true;
  } 
  else if( (amplitude != 0xFF) && (ampl_old != amplitude) )
  {
    music_effect_mem[3] = amplitude;
    
    retVal = true;
  }
  
  return retVal;
}

void LEDMusicEffects::print_amplitude_static(uint8_t led_start, uint8_t led_end, const bool top, uint8_t amplitude, uint8_t r_base, uint8_t g_base, uint8_t b_base)
{
  
  /* Check if the base color has been changed  */
  if(is_update(r_base,g_base,b_base,0xFF))
  {
    /* Generate new static colors based on the base color */
     generate_static_colors(r_base,g_base,b_base);
  }
 
  if(top)
  {    
    for(uint8_t i = led_start; i < amplitude; i++)
    {
      m_leds[i] = m_static_color[i];
    }
    for(uint8_t i = amplitude; i < led_end; i++)
    {
      m_leds[i] = CRGB(0,0,0);
    }
  }
  else
  {
    for(uint8_t i = led_start; i < (led_end - amplitude); i++)
    {
      m_leds[i] = CRGB(0,0,0);
    }
    for(uint8_t i = (led_end - amplitude); i < led_end; i++)
    {
      m_leds[i] = m_static_color[i];
    }    
  }
   FastLED.show();  
} 
  

void LEDMusicEffects::power_bars_effect(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude, DirectionType direction, EffectType effect_type)
{
  unsigned long now = m_timer->getTime();

  /* Time to update the LED status */
  if( (now - m_last_iteration) > print_delay )
  {
    /* Reset timer */
    m_last_iteration = now;
    
    /* If there is no update since last iteration, perform a decay effect by reducing the requested amplitude one unit */
    if( !is_update(r,g,b,amplitude) )
    {
      amplitude--;
    }      

    if(effect_type == COLOR)
    {
      if(direction == UP)
      {
        print_amplitude_color(0, NUM_LEDS, true, amplitude, r, g, b);
      }
      else if(direction == DOWN)
      {
        print_amplitude_color(0, NUM_LEDS, false, amplitude, r, g, b);
      }
      else if(direction == MIDDLE)
      {
        print_amplitude_color(NUM_LEDS / 2, NUM_LEDS, true, amplitude / 2, r, g, b);
        print_amplitude_color(0, NUM_LEDS / 2, false , amplitude / 2, r, g, b);
      }
    }
    else if(effect_type == STATIC)
    {
      if(direction == UP)
      {
        print_amplitude_static(0, NUM_LEDS, true, amplitude, r, g, b);
      }
      else if(direction == DOWN)
      {
        print_amplitude_static(0, NUM_LEDS, false, amplitude, r, g, b);
      }
      else if(direction == MIDDLE)
      {
        print_amplitude_static(NUM_LEDS / 2, NUM_LEDS, true, amplitude / 2, r, g, b);
        print_amplitude_static(0, NUM_LEDS / 2, false , amplitude / 2, r, g, b);
      }
    }
  }
}


/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
