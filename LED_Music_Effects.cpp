
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

LEDMusicEffects::hsv LEDMusicEffects::rgb2hsv(RGBcolor in_rgb)
{

    hsv         in;
    hsv         out;
    double      min, max, delta;

    /* Adapt format */
    in.h = ((double)in_rgb.R) / 255;
    in.s = ((double)in_rgb.G) / 255;
    in.v = ((double)in_rgb.B) / 255;

    min = in.h < in.s ? in.h : in.s;
    min = min  < in.v ? min  : in.v;

    max = in.h > in.s ? in.h : in.s;
    max = max  > in.v ? max  : in.v;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.h >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.s - in.v ) / delta;        // between yellow & magenta
    else
    if( in.s >= max )
        out.h = 2.0 + ( in.v - in.h ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.h - in.s ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}

RGBcolor LEDMusicEffects::hsv2rgb(LEDMusicEffects::hsv in)
{
  double      hh, p, q, t, ff;
  long        i;
  hsv         out; //same datatype to buffer the output
  RGBcolor retVal;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.h = in.v;
        out.s = in.v;
        out.v = in.v;

        retVal.R = (uint8_t)(255*out.h);
        retVal.G = (uint8_t)(255*out.s);
        retVal.B = (uint8_t)(255*out.v);
        
        return retVal;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.h = in.v;
        out.s = t;
        out.v = p;
        break;
    case 1:
        out.h = q;
        out.s = in.v;
        out.v = p;
        break;
    case 2:
        out.h = p;
        out.s = in.v;
        out.v = t;
        break;

    case 3:
        out.h = p;
        out.s = q;
        out.v = in.v;
        break;
    case 4:
        out.h = t;
        out.s = p;
        out.v = in.v;
        break;
    case 5:
    default:
        out.h = in.v;
        out.s = p;
        out.v = q;
        break;
    }

    retVal.R = (uint8_t)(255*out.h);
    retVal.G = (uint8_t)(255*out.s);
    retVal.B = (uint8_t)(255*out.v);
    
    return retVal;     
}

void LEDMusicEffects::shift_leds(uint8_t led_start, uint8_t led_end, uint8_t positions, const bool top, const uint8_t delay_ms, const uint8_t R_in, const uint8_t G_in, const uint8_t B_in)
{
  if(led_end <= led_start) return;
  
  if(positions == 0) positions = 1;
  if( (led_end - led_start) < positions ) positions = led_end - led_start;
  
  if(top)
  {
    for(uint8_t j = led_end - 1; j >= (led_start + positions) ; j--)
    {
      m_leds[j] = m_leds[j-positions];
      //Serial.print("Position shift up: ");
      //Serial.println(j);
      //delay(50);
    }
    for(uint8_t j = led_start; j < (led_start + positions); j++)
    {
      m_leds[j] = CRGB(R_in,G_in,B_in);
      //Serial.print("Position fill up: ");
      //Serial.println(j);
      //delay(50);
    }
  }

  else 
  {
    for(uint8_t j = led_start; j < (led_end - positions); j++)
    {
      m_leds[j] = m_leds[j+positions];
      //Serial.print("Position shift down: ");
      //Serial.println(j);
      //delay(50);
    }
    for(uint8_t j = led_end - positions; j < led_end; j++)
    {
      m_leds[j] = CRGB(R_in,G_in,B_in);
      //Serial.print("Position fill down: ");
      //Serial.println(j);
      //delay(50);
    }
  }

  FastLED.show();
  delay(delay_ms);
}

uint8_t LEDMusicEffects::compute_bubble_amplitude(uint8_t amplitude)
{
  uint8_t adaptedAmplitude;
  
  if(amplitude > 75) adaptedAmplitude = 5;
  else if(amplitude > 50) adaptedAmplitude = 4;
  else if(amplitude > 40) adaptedAmplitude = 3;
  else if(amplitude > 20) adaptedAmplitude = 2;
  else adaptedAmplitude = 1;

  return adaptedAmplitude;
}

void LEDMusicEffects::bubble_effect(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude, uint8_t direction)
{
  unsigned long now = m_timer->getTime();

  if( (now - m_last_iteration) > print_delay )
  {
#if 0
    Serial.println("Running bubble effect. ");
    Serial.print("Color: ");
    Serial.print(r);
    Serial.print(" ");
    Serial.print(g);
    Serial.print(" ");
    Serial.println(b);
    Serial.print("Dir: ");
    Serial.println(direction);
#endif

    m_last_iteration = now;

    /* Adapt the amplitude from a range [0-100] to a number of leds to shift */
    amplitude = compute_bubble_amplitude(amplitude);
    
    if(direction == 0) //Up 
    {
      shift_leds(0, NUM_LEDS, amplitude, true, 0, r, g, b);
    }
    else if(direction == 1) //Down
    {
      shift_leds(0, NUM_LEDS, amplitude, false, 0, r, g, b);
    }
    else if(direction == 2) //Middle-out
    {
      shift_leds(0, NUM_LEDS / 2 , amplitude, false, 0, r, g, b);
      shift_leds(NUM_LEDS / 2, NUM_LEDS, amplitude, true, 0, r, g, b); 
    }
    else if(direction == 3) //Out-middle
    {
      shift_leds(0, NUM_LEDS / 2 , amplitude, true, 0, r, g, b);
      shift_leds(NUM_LEDS / 2, NUM_LEDS, amplitude, false, 0, r, g, b);
      
    }
  }
}

void LEDMusicEffects::print_amplitude_color(uint8_t led_start, uint8_t led_end, const bool top, uint8_t amplitude, uint8_t r, uint8_t g, uint8_t b)
{
  
  if(top)
  {    
    for(uint8_t i = led_start; i < led_start + amplitude; i++)
    {
      m_leds[i] = CRGB(r,g,b);
    }
    for(uint8_t i = led_start + amplitude; i < led_end; i++)
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


void LEDMusicEffects::generate_static_colors(uint8_t r_base, uint8_t g_base, uint8_t b_base, uint8_t increment)
{
  
  hsv hsv_color;
  RGBcolor rgb_color;

  rgb_color.R = r_base;
  rgb_color.G = g_base;
  rgb_color.B = b_base;

#if 0
  Serial.print("Color base: ");
  Serial.print(r_base);
  Serial.print(" ");
  Serial.print(g_base);
  Serial.print(" ");
  Serial.println(b_base);

  Serial.print("Increment: ");
  Serial.println(increment);
#endif
  
  /* Translate the base color from RGB to HSV */
  hsv_color = rgb2hsv(rgb_color);

#if 0
  Serial.print("HSV base: ");
  Serial.print(hsv_color.h);
  Serial.print(" ");
  Serial.print(hsv_color.s);
  Serial.print(" ");
  Serial.println(hsv_color.v);
#endif

  /* Set S and V to the maximum value */
  //hsv_color.s = 1.0;
  //hsv_color.v = 1.0;

  /* Increment the Hue for each led */
  for(uint8_t i = 0; i < NUM_LEDS; i++)
  {
    hsv_color.h += increment;
    if(hsv_color.h > 360.0) hsv_color.h = 0.0; //Circular increment between [0,360]
    /* Convert to RGB */
    rgb_color = hsv2rgb(hsv_color);

#if 0
    Serial.print("RGB[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.print(rgb_color.R);
    Serial.print(" ");
    Serial.print(rgb_color.G);
    Serial.print(" ");
    Serial.println(rgb_color.B);
#endif
    
    /* Add color to the led shadow memory */
    m_static_color[i] = CRGB(rgb_color.R,rgb_color.G,rgb_color.B);
  }
  
}

bool LEDMusicEffects::is_update(uint8_t r, uint8_t g, uint8_t b, uint8_t amplitude)
{
  bool retVal = false;
  
  uint8_t r_old = music_effect_mem[0];
  uint8_t g_old = music_effect_mem[1];
  uint8_t b_old = music_effect_mem[2];
  uint8_t ampl_req = music_effect_mem[3];
  
  if( (r_old != r) || (g_old != g) || (b_old != b) )
  {
    /* Update effect memory */
    music_effect_mem[0] = r;
    music_effect_mem[1] = g;
    music_effect_mem[2] = b;
    
    retVal = true;
  } 
  else if( (amplitude != 0xFF) && (ampl_req != amplitude) )
  {
    music_effect_mem[3] = amplitude;
    
    retVal = true;
  }
  
  return retVal;
}

void LEDMusicEffects::print_amplitude_static(uint8_t led_start, uint8_t led_end, const bool top, uint8_t amplitude, uint8_t r_base, uint8_t g_base, uint8_t b_base, uint8_t increment)
{
  
  /* Check if the base color has been changed  */
  if(is_update(r_base,g_base,b_base,0xFF))
  {
    Serial.println("New color update. Generating static colors");
    /* Generate new static colors based on the base color */
     generate_static_colors(r_base,g_base,b_base,increment);
  }
 
  if(top)
  {  
   
    for(uint8_t i = led_start; i < led_start + amplitude; i++)
    {
      m_leds[i] = m_static_color[i];
    }
    for(uint8_t i = led_start + amplitude; i < led_end; i++)
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
  

void LEDMusicEffects::power_bars_effect(uint32_t print_delay, uint8_t r, uint8_t g, uint8_t b, uint8_t& amplitude, uint8_t direction, uint8_t effect_type, uint8_t increment)
{
  unsigned long now = m_timer->getTime();

  /* Time to update the LED status */
  if( (now - m_last_iteration) > print_delay )
  {
    #if 0
    Serial.println("Running power bars effect. ");
    Serial.print("Color: ");
    Serial.print(r);
    Serial.print(" ");
    Serial.print(g);
    Serial.print(" ");
    Serial.println(b);
    Serial.print("Requested amplitude: ");
    Serial.println(amplitude);
    Serial.print("Dir: ");
    Serial.print(direction);
    Serial.print(" Effect ");
    Serial.println(effect_type);
    #endif
    
    /* Reset timer */
    m_last_iteration = now;

    if(effect_type == 0)
    {
      if(direction == 0) //Up
      {
        print_amplitude_color(0, NUM_LEDS, true, amplitude, r, g, b);
      }
      else if(direction == 1) //Down
      {
        print_amplitude_color(0, NUM_LEDS, false, amplitude, r, g, b);
      }
      else if(direction == 2) //Middle-out
      {
        print_amplitude_color(NUM_LEDS / 2, NUM_LEDS, true, amplitude / 2, r, g, b);
        print_amplitude_color(0, NUM_LEDS / 2, false , amplitude / 2, r, g, b);
      }
      else if(direction == 3) //Out-middle
      {
        print_amplitude_color(NUM_LEDS / 2, NUM_LEDS, false, amplitude / 2, r, g, b);
        print_amplitude_color(0, NUM_LEDS / 2, true , amplitude / 2, r, g, b);
      }
    }
    else if(effect_type == 1)
    {
      if(direction == 0) //Up
      {
        print_amplitude_static(0, NUM_LEDS, true, amplitude, r, g, b, increment);
      }
      else if(direction == 1) //Down
      {
        print_amplitude_static(0, NUM_LEDS, false, amplitude, r, g, b, increment);
      }
      else if(direction == 2) //Middle-out
      {
        print_amplitude_static(NUM_LEDS / 2, NUM_LEDS, true, amplitude / 2, r, g, b, increment);
        print_amplitude_static(0, NUM_LEDS / 2, false , amplitude / 2, r, g, b, increment);
      }
      else if(direction == 3) //Out-middle
      {
        print_amplitude_static(NUM_LEDS / 2, NUM_LEDS, false, amplitude / 2, r, g, b, increment);
        print_amplitude_static(0, NUM_LEDS / 2, true , amplitude / 2, r, g, b, increment);
      }
    }

    /* If there is no update since last iteration, perform a decay effect by reducing the requested amplitude one unit */
    if( !is_update(r,g,b,amplitude) )
    {
      if(amplitude != 0) amplitude--;     
    }
  }
}


/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
