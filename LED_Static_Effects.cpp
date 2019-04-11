
#include "LED_Static_Effects.h"


LEDStaticEffects::LEDStaticEffects(LEDController* led_controller, CRGB* leds_ptr, timeSync* timer):
m_led_controller(led_controller),
leds(leds_ptr),
m_timer(timer)
{
  start_sequence = true;
  m_last_iteration = 0;
}

void LEDStaticEffects::end_effect()
{
  start_sequence = true;
}

void LEDStaticEffects::RGBLoop()
{  

  unsigned long now = m_timer->getTime();
  static const uint8_t delay_ms = 3;

  if( (now - m_last_iteration) > delay_ms )
  {
    m_last_iteration = now;
    int j,k,order;
    if(start_sequence)
    {
      j = 0;
      k = 0;
      order = 0;
      start_sequence = false;  
    }
    else
    {
      j = m_effect_state[0];
      k = m_effect_state[1];  
      order = m_effect_state[2];    
    }
    switch(j) 
    {
      case 0: setAll(k,0,0); break;
      case 1: setAll(0,k,0); break;
      case 2: setAll(0,0,k); break;
    }
    showStrip();

    if(order == 0)
    {
      k++;
      if(k == 255) order = 1;
    }
    else
    {
      k--;
      if(k == 0)
      {
        order = 0;
        ++j >= 3 ? j = 0 : j = j;
      }
    }

    m_effect_state[0] = j;
    m_effect_state[1] = k;
    m_effect_state[2] = order;
  }    
}

void LEDStaticEffects::FadeInOut(byte red, byte green, byte blue)
{

  int k,order;
  if(start_sequence)
  {
    k = 0;
    order = 0;
    start_sequence = false;  
  }
  else
  {
    k = m_effect_state[0];
    order = m_effect_state[1];     
  }
  float r, g, b;

  if(order == 0)
  {
    k++;

    if(k == 255) order = 1;
  }
  else
  {
    k--;
    if(k == 0) order = 0;
  }

  r = int((float)((k/256.0))*red);
  g = int((float)((k/256.0))*green);
  b = int((float)((k/256.0))*blue);

  setAll(r,g,b);
  showStrip();

  m_effect_state[0] = k;
  m_effect_state[1] = order;  
}

void LEDStaticEffects::Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause)
{

  unsigned long now = m_timer->getTime();
  int j,order;
  if(start_sequence)
  {
    j = 0;    
    order = 0;
    start_sequence = false;  
  }
  else
  {
    j = m_effect_state[0];
    order = m_effect_state[1];    
  }

  if( ((now - m_last_iteration) > FlashDelay) && ( (order == 0) || (order == 1) ) )
  {
    m_last_iteration = now;    

    if(order == 0)
    {
      setAll(red,green,blue);
      showStrip();
      order = 1;
    }
    else if(order == 1)
    {
      setAll(0,0,0);
      showStrip();
      if(++j < StrobeCount) order = 0;
      else
      {
        j = 0;
        order = 2;
      }
    }
  }

  else if ( ((now - m_last_iteration) > EndPause) && (order == 2)  )
  {
    order = 0;
  }


  m_effect_state[0] = j;
  m_effect_state[1] = order;    
}

// Apply LED color changes
void LEDStaticEffects::showStrip()
{
#ifdef ADAFRUIT_NEOPIXEL_H
   // NeoPixel
   strip.show();
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
#endif 
}
  
// Set a LED color (not yet visible)
void LEDStaticEffects::setPixel(int Pixel, byte red, byte green, byte blue)
{
 #ifdef ADAFRUIT_NEOPIXEL_H
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif 
}

// Set all LEDs to a given color and apply it (visible)
void LEDStaticEffects::setAll(byte red, byte green, byte blue)
{
  for(int i = 0; i < NUM_LEDS; i++ ) 
  {
    setPixel(i, red, green, blue);
  }
  showStrip();
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
