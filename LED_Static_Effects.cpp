
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
    Serial.println("Running Strobe");
    Serial.println(StrobeCount);
    Serial.println(FlashDelay);
    Serial.println(EndPause);
    order = 0;
  }


  m_effect_state[0] = j;
  m_effect_state[1] = order;    
}

void LEDStaticEffects::Fire(int Cooling, int Sparking, int SpeedDelay)
{

  //Serial.println("Running Fire");
  //Serial.println(SpeedDelay);  
  
  static byte heat[NUM_LEDS];

  unsigned long now = m_timer->getTime();   

  if((now - m_last_iteration) > SpeedDelay) 
  {
    
    m_last_iteration = now;       
    int cooldown;
  
    // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
  
      if(cooldown>heat[i]) {
        heat[i]=0;
      } else {
        heat[i]=heat[i]-cooldown;
      }
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }
  
    // Step 3.  Randomly ignite new 'sparks' near the bottom
    if( random(255) < Sparking ) {
      int y = random(7);
      heat[y] = heat[y] + random(160,255);
      //heat[y] = random(160,255);
    }
  
    // Step 4.  Convert heat to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      setPixelHeatColor(j, heat[j] );
    }
  
    showStrip();
  }
    
}

void LEDStaticEffects::setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}
  
void LEDStaticEffects::HalloweenEyes(byte red, byte green, byte blue, int EyeWidth, int EyeSpace, bool Fade, int Steps, int FadeDelay, int EndPause)
{
  unsigned long now = m_timer->getTime();
  int j,order;
  if(start_sequence)
  {
    j = Steps;
    order = 0;
    start_sequence = false;  
  }
  else
  {
    j = m_effect_state[0]; 
    order = m_effect_state[1];    
  }

  randomSeed(analogRead(0));  
   
  int StartPoint  = random( 0, NUM_LEDS - (2*EyeWidth) - EyeSpace );
  int Start2ndEye = StartPoint + EyeWidth + EyeSpace;     

  if ( ((now - m_last_iteration) > EndPause) && (order == 1)  )
  {
    for(int i = 0; i < EyeWidth; i++) 
    {
      setPixel(StartPoint + i, red, green, blue);
      setPixel(Start2ndEye + i, red, green, blue);
    }    
  }

  if( ((now - m_last_iteration) > FadeDelay) &&  (order == 0) )
  {
    m_last_iteration = now;     
  
    showStrip();
  
    if(Fade==true) 
    {
      float r, g, b;  
      
      r = j*(red/Steps);
      g = j*(green/Steps);
      b = j*(blue/Steps);

      for(int i = 0; i < EyeWidth; i++) 
      {
        setPixel(StartPoint + i, r, g, b);
        setPixel(Start2ndEye + i, r, g, b);
      }

      showStrip();

      if(j-- == 0)
      {
        j = Steps;
        order = 1;
      }     
    }
  }

  else if ( ((now - m_last_iteration) > EndPause) && (order == 1)  )
  {
    setAll(0,0,0); // Set all black
    order = 0;
  }

  m_effect_state[0] = j;
  m_effect_state[1] = order;    
}
  
void LEDStaticEffects::CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
  Serial.println("Running CyclonBounce");
  Serial.println(EyeSize);
  Serial.println(SpeedDelay);
  Serial.println(ReturnDelay);

  
  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) 
  {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) 
    {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) 
  {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) 
    {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);
}
  
void LEDStaticEffects::NewKITT(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
  Serial.println("Running NEW KITT");
  Serial.println(red);
  Serial.println(green);
  Serial.println(blue);
  Serial.println(EyeSize);
  Serial.println(SpeedDelay);
  Serial.println(ReturnDelay);
  
  RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
}

// used by NewKITT
void LEDStaticEffects::CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) 
{
  Serial.println("Running Center To Outside");
  for(int i =((NUM_LEDS-EyeSize)/2); i>=0; i--) 
  {
    setAll(0,0,0);

    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) 
    {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);

    setPixel(NUM_LEDS-i-1, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) 
    {
      setPixel(NUM_LEDS-i-j, red, green, blue);
    }
    setPixel(NUM_LEDS-i-EyeSize-1, red/10, green/10, blue/10);

    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void LEDStaticEffects::OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  
  Serial.println("Running Outside To Center");
  
  for(int i = 0; i<=((NUM_LEDS-EyeSize)/2); i++)
  {
    setAll(0,0,0);

    //Serial.println(i);

    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) 
    {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);

    setPixel(NUM_LEDS-i-1, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++)
    {
      setPixel(NUM_LEDS-i-j, red, green, blue);
    }
    setPixel(NUM_LEDS-i-EyeSize-1, red/10, green/10, blue/10);

    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void LEDStaticEffects::LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  
  Serial.println("Running Left to Right");
  
  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) 
  {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) 
    {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void LEDStaticEffects::RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) 
{

  Serial.println("Running Right To Left");

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) 
  {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    
    for(int j = 1; j <= EyeSize; j++) 
    {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
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
  if(Pixel >= NUM_LEDS)
  {
    Serial.println("ERROR: Led memory size exceeded");
    Serial.print("Idx ");
    Serial.println(Pixel);
    Pixel = NUM_LEDS - 1;
  }

  
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
