
#include "LED_controller.h"


LEDController::LEDController(lamp_status* lamp_status_request):
m_lamp_status_request(lamp_status_request)
{
  m_static_effects = new LEDStaticEffects(this, m_leds, &m_timer);
}

LEDController::~LEDController()
{
  delete m_static_effects;
}

void LEDController::setRGB(RGBcolor color)
{
  setAllLeds(color,0);
}

void LEDController::resync()
{
  m_last_iteration = m_timer.getTime();
  led_idx = 0;
  print_task = 0;  
}

void LEDController::setRGB(uint8_t R, uint8_t G, uint8_t B)
{
  RGBcolor color_req;
  color_req.R = R;
  color_req.G = G;
  color_req.B = B;
  setAllLeds(color_req,0);
}

void LEDController::setup()
{
  FastLED.addLeds<WS2812, LED_PIN, GRB>(m_leds, NUM_LEDS);
  
  setRGB(0,0,0);
}

void LEDController::setAllLeds(RGBcolor color, unsigned long delay_ms)
{
  for(uint8_t i = 0; i < NUM_LEDS; i++ )
  {
    m_leds[i] = CRGB(color.R, color.G, color.B);
    FastLED.show();
    delay(delay_ms); 
  }
}

void LEDController::update_color()
{
  setAllLeds(m_lamp_status_request->color,0);
}
  
void LEDController::update_brightness()
{
  
}

void LEDController::feed()
{
  /* Music effects */
  if(m_mode > 1 && m_mode < 10)
  {
    switch(m_mode)
    {
      case 2:
        test_effect();
        break;
    }
  }

  /* Static effects */
  else if(m_mode >= 10)
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
        m_static_effects->Strobe(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B,50, 30, 500);
        break;
      default:
        break;      
    }
  }
  
  
}
  
void LEDController::update_mode()
{
  m_mode = m_lamp_status_request->lamp_mode;

  switch(m_mode)
  {
    case 0:
      setRGB(0,0,0);
      break;
    case 1:
      setRGB(20,20,20);
      break;
  }
}

void LEDController::end_effect()
{
  m_static_effects->end_effect();
}

void LEDController::test_effect()
{
  unsigned long now = m_timer.getTime();

  if( ((now - m_last_iteration) > PRINT_DELAY) && (print_task == 0) )
  {
    //Serial.println("Iteration ON due");
    m_last_iteration = now;
    if( led_idx < NUM_LEDS )
    {
      m_leds[led_idx] = CRGB(m_lamp_status_request->color.R, m_lamp_status_request->color.G, m_lamp_status_request->color.B);
      FastLED.show();
      led_idx++;
    }
    else
    {
      led_idx = 0;
      print_task = 1;
    }
  }
  
  else if( ((now - m_last_iteration) > PRINT_DELAY) && (print_task == 1) )
  {
    //Serial.println("Iteration OFF due");
    m_last_iteration = now;
    if( led_idx < NUM_LEDS )
    {
      m_leds[led_idx] = CRGB(0, 0, 0);
      FastLED.show();
      led_idx++;
    }
    else
    {
      led_idx = 0;
      print_task = 0;
    }    
  }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
