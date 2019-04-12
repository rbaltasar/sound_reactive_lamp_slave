
#if !defined LEDSTATICEFFECTS_H
#define LEDSTATICEFFECTS_H

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

class LEDStaticEffects
{

private:

  LEDController* m_led_controller;
  CRGB* leds;
  timeSync* m_timer;

  int m_effect_state[3];
  bool start_sequence;
  unsigned long m_last_iteration;
  
  // Apply LED color changes
  void showStrip();
  // Set a LED color (not yet visible)
  void setPixel(int Pixel, byte red, byte green, byte blue);
  // Set all LEDs to a given color and apply it (visible)
  void setAll(byte red, byte green, byte blue);
  void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
  void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
  void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
  void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
  void setPixelHeatColor (int Pixel, byte temperature);

public: 

  LEDStaticEffects(LEDController* led_controller, CRGB* leds_ptr, timeSync* timer);

  void end_effect();

  void RGBLoop();
  void FadeInOut(byte red, byte green, byte blue);
  void Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause);
  void Fire(int Cooling, int Sparking, int SpeedDelay);
  void HalloweenEyes(byte red, byte green, byte blue, int EyeWidth, int EyeSpace, bool Fade, int Steps, int FadeDelay, int EndPause);
  void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
  void NewKITT(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);

#if 0
  void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
  void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
  void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
  void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
  void Twinkle(byte red, byte green, byte blue, int Count, int SpeedDelay, bool OnlyOne);
  void TwinkleRandom(int Count, int SpeedDelay, bool OnlyOne);
  void Sparkle(byte red, byte green, byte blue, int SpeedDelay);
  void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay);
  void RunningLights(byte red, byte green, byte blue, int WaveDelay);
  void colorWipe(byte red, byte green, byte blue, int SpeedDelay);
  void rainbowCycle(int SpeedDelay);
  byte * Wheel(byte WheelPos);
  void theaterChase(byte red, byte green, byte blue, int SpeedDelay);
  void theaterChaseRainbow(int SpeedDelay);  
  void setPixelHeatColor (int Pixel, byte temperature);
  void BouncingColoredBalls(int BallCount, byte colors[][3], bool continuous, const uint8_t background);
  void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, bool meteorRandomDecay, int SpeedDelay, bool dir = true);
  void fadeToBlack(int ledNo, byte fadeValue);
#endif
  
};

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
