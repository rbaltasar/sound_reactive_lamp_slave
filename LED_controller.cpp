
#include "LED_controller.h"

LEDController::LEDController(lamp_status* lamp_status_request, timeSync* timer):
m_lamp_status_request(lamp_status_request),
m_timer(timer)
{
  /* Create static effects object */
  m_static_effects = new LEDStaticEffects(this, m_leds, m_timer);
  /* Create music effects object */
  m_music_effects = new LEDMusicEffects(this, m_leds, m_timer);
}

LEDController::~LEDController()
{
  /* Deallocate memory */
  delete m_static_effects;
  delete m_music_effects;
}

/* Do timer resynchronization for static and music effects */
void LEDController::resync()
{
  m_static_effects->resync();
  m_music_effects->resync();
  
}

/* Hardware setup */
void LEDController::setup()
{
  /* Configure LED stripe */
  FastLED.addLeds<WS2812, LED_PIN, GRB>(m_leds, NUM_LEDS);
  
  /* Reset color */
  setAllLeds( (RGBcolor){0,0,0}, 0 );
}

/* Set all leds with the same color */
void LEDController::setAllLeds(RGBcolor color, unsigned long delay_ms)
{
  setLeds(color,delay_ms,NUM_LEDS);
}

/* Set a defined number of LEDs with the same color, with or without erasing the non-set leds */
void LEDController::setLeds(RGBcolor color, unsigned long delay_ms, uint8_t num_leds, bool erase_others)
{
  /* Clip value */
  color.R > 255 ? color.R = 255 : color.R = color.R;
  color.G > 255 ? color.G = 255 : color.G = color.G;
  color.B > 255 ? color.B = 255 : color.B = color.B;
  /* Set requested leds */
  for(uint8_t i = 0; i < num_leds; i++ )
  {
    m_leds[i] = CRGB(color.R, color.G, color.B);
    FastLED.show();
    delay(delay_ms); 
  }
  /* Eras non-requested led */
  if(erase_others)
  {
    for(uint8_t i = num_leds; i < NUM_LEDS; i++ )
    {
      m_leds[i] = CRGB(0, 0, 0);
    }
    FastLED.show();
  }
}

/* Update the color for all leds */
void LEDController::update_color()
{
  setAllLeds(m_lamp_status_request->color,0);
} 

/* Update the LED stripe for static and dynamic effects */
void LEDController::feed()
{
  /* Static effects - Range [10,99] */
  if(m_mode >= 10 && m_mode < 99)
  {
    switch(m_mode - 10)
    {
      case RGB_LOOP:
        m_static_effects->RGBLoop();
        break;
      case FADE_IN_OUT:
        m_static_effects->FadeInOut(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B);
        break;
      case STROBE:
        m_static_effects->Strobe(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B,50, m_lamp_status_request->effect_speed, m_lamp_status_request->effect_delay);
        break;
      case FIRE:
        m_static_effects->Fire(55, 120,m_lamp_status_request->effect_speed);
        break;
      case HALLOWEEN_EYES:
        m_static_effects->HalloweenEyes(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, NUM_LEDS/5, 2, true, 1, m_lamp_status_request->effect_speed,  m_lamp_status_request->effect_delay);
        break;         
      case CYCLON_BOUNCE:
        m_static_effects->CylonBounce(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, NUM_LEDS/5, m_lamp_status_request->effect_speed, m_lamp_status_request->effect_delay);
        break;
      case TWINKLE:
        m_static_effects->Twinkle(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, 16, m_lamp_status_request->effect_speed, false);
        break;
      case TWINKLE_RANDOM:
        m_static_effects->TwinkleRandom(16, m_lamp_status_request->effect_speed, false);
        break;
      case SPARKLE:
        m_static_effects->Sparkle(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->effect_speed);
        break;
      case SNOW_SPARKLE:
        m_static_effects->SnowSparkle(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->effect_speed, m_lamp_status_request->effect_delay);
        break;
      case RUNNING_LIGHTS:
        m_static_effects->RunningLights(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->effect_speed);
        break;
      case COLOR_WIPE:
        m_static_effects->colorWipe(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->effect_speed);
         break;
      case RAINBOW_CYCLE:
         m_static_effects->rainbowCycle(m_lamp_status_request->effect_speed);
         break;
      case THEATER_CHASE:
         m_static_effects->theaterChase(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->effect_speed);
         break;
      case THEATER_CHASE_RAINBOW:
         m_static_effects->theaterChaseRainbow(m_lamp_status_request->effect_speed);
         break;
      case BOUNCING_COLORED_BALLS:
      {
         byte color[10][3] = { {m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B},
                                    {m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->color.R},
                                    {m_lamp_status_request->color.B, m_lamp_status_request->color.R, m_lamp_status_request->color.G},
                                    {m_lamp_status_request->color.R, m_lamp_status_request->color.R, m_lamp_status_request->color.G},
                                    {m_lamp_status_request->color.G, m_lamp_status_request->color.G, m_lamp_status_request->color.R},
                                    {m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.R},
                                    {m_lamp_status_request->color.B, m_lamp_status_request->color.B, m_lamp_status_request->color.G},
                                    {m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.R},
                                    {m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->color.G},
                                    {m_lamp_status_request->color.R, m_lamp_status_request->color.B, m_lamp_status_request->color.B}};
         m_static_effects->BouncingColoredBalls(m_lamp_status_request->effect_amount, color, true, 2);
         break;
      }
      case METEOR_RAIN:
         m_static_effects->meteorRain(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->effect_amount, m_lamp_status_request->effect_amount / 2, true, m_lamp_status_request->effect_speed);
         break;
      case FADE_TO_BLACK:
         m_static_effects->fadeToBlack(20/*int ledNo*/, 0x0F/*byte fadeValue*/);
         break;
      default:
        break;      
    }
  }
  /* Music effects - Range >= 100 */
  else if(m_mode >= 100)
  {
     switch(m_mode - 100)
    {
      case BUBBLE:
        m_music_effects->bubble_effect(m_lamp_status_request->effect_delay, m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->amplitude, m_lamp_status_request->effect_direction);
        break;
      case ENERGY_BAR:
        m_music_effects->power_bars_effect(m_lamp_status_request->effect_delay, m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->amplitude, m_lamp_status_request->effect_direction, 0, 0);
        break;
      case ENERGY_BAR_COLOR:
        m_music_effects->power_bars_effect(m_lamp_status_request->effect_delay, m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B, m_lamp_status_request->amplitude, m_lamp_status_request->effect_direction, 1, m_lamp_status_request->color_increment);
        break;
    }
  }
}
  
/* Update the LED mode */
void LEDController::update_mode()
{
  /* Set local mode */
  m_mode = m_lamp_status_request->lamp_mode;

  /* Handle special cases (no effect) - Range[0,9] */
  switch(m_mode)
  {
    case 0: //Switch Off
      setAllLeds( (RGBcolor){0,0,0}, 0 );
      break;
    case 1: //Switch On (with brightness
      setAllLeds( (RGBcolor){m_lamp_status_request->color.R / m_lamp_status_request->brightness,m_lamp_status_request->color.G / m_lamp_status_request->brightness,m_lamp_status_request->color.B / m_lamp_status_request->brightness}, 0 );
      break;
    case 4: //Ambient light effect
      ambient_light_effect();
      break;    
  }
}

/* Cleanup and finish current effect */
void LEDController::end_effect()
{
  m_static_effects->end_effect();
  m_music_effects->end_effect();
}

/* Ambient light effect
*  The intensity of the light depends on the ambient light, provided by an external node
*/
void LEDController::ambient_light_effect()
{ 
  RGBcolor targetColor;
  /* Too much light. Shut down the lamp */
  if(m_lamp_status_request->light_amount > AMBIENT_LIGHT_THRESHOLD)
  {
    targetColor.R = 0;
    targetColor.G = 0;
    targetColor.B = 0;
  }
  /* Brightness depends on the light amount */
  else
  {
    float multiplier;

    if(m_lamp_status_request->light_amount > (AMBIENT_LIGHT_THRESHOLD / 2))
    {
      multiplier = (AMBIENT_LIGHT_THRESHOLD - m_lamp_status_request->light_amount) / (4*AMBIENT_LIGHT_THRESHOLD);
    }
    else if (m_lamp_status_request->light_amount > 0)
    {
      multiplier = (AMBIENT_LIGHT_THRESHOLD - m_lamp_status_request->light_amount) / (2*AMBIENT_LIGHT_THRESHOLD);
    }
    else
    {
      multiplier = 1;
    }

    targetColor.R = R_DEFAULT * multiplier;
    targetColor.G = G_DEFAULT * multiplier;
    targetColor.B = B_DEFAULT * multiplier;
  }

  /* Change smoothly to the target color */
  m_static_effects->fade_to_color(targetColor,AMBIENT_LIGHT_DELAY); 
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
