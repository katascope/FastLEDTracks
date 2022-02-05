// FastLEDTracks : A time-track event system for FastLED
// Author: Layne Thomas
// Purpose is for a wearable dance-synchronized light-suit.
// Uses:
//  FastLED for addressable LED
//  SoftwareSerial for BlueTooth
//  Fx for LED effects
//  Track for track system
/*
 * Design Criteria:
 *  Audioreactive : The device must represent the dance music.
 *  Lead-Follow : The follow device can synchronize with the lead device.
 *  Simple : The device must be extremely easy to use once setup.
 *  Reliable : The software must work correctly during the dance.
 *   Care with overloading bluetooth buffer
 *   Startup 3 seconds for led safety
 *  Minimal : The device must strenously optimize RAM usage.
 *   PROGMEM for Track - Can expand without using RAM
 *   See: https://www.arduino.cc/reference/en/language/variables/utilities/progmem/
 *  Configurable : The code must be highly configurable by #DEFINE.
 *  Recoverable : The device should be able to recover from mistimings.
 *  Cheap : The device must be low cost and easy to produce.
 *  Portable : The device must be human wearable and untethered.
 *  Stable : The device must withstand repeated heavy use dancing.
 *  Subtle : Indirect lighting for led's, hazed frosted fogged.
 *  Compatible : The device must maximize use of standards.
 *   Arduino Nano : Standard microcontroller kit
 *   WS2811 : Addressable LED wire : Standard 3-wire connectors
 *   BTF-Lighting Connectors : Addressable 3-wire LED wire connectors
 *   Also used for power connector into arduino.
 *   XT-60 Battery connector with D-Sun BEC Voltage convert (3-cell 12v down to 5v)
 *    or 18650 Battery Shield case using 5-V output
 *   Thingiverse Case & plans
 *   ffmpeg for Spectrum Analysis
 *   HC-05 Bluetooth https://www.evelta.com/blog/instructions-to-set-bluetooth-module-hc05-password-using-arduino/
 *    AT+NAME?
 *    AT+NAME="LedLight"
 *    AT+PSWD="3838"
 *   Master+Slave BT bindings: https://www.instructables.com/Arduino-Bluetooth-Master-and-Slave-Using-Any-HC-05/
 * Tradeoffs:
 *   Audio track setup does not have to be easy or quick.
 *   Different arduinos can be used.
 *   Different forms of power/battery can be used.
 *   Arduino Micro Pro can be used if possible instead of Arduino Nano
 * Aesthetic Criteria:
 *   Tasteful : Use complex colors, avoid 'rgb rainbow land'
 *   Smooth : Fading color transitions over palette rotations
 *   Fade-To interpolated timing so endings match startings
 * To create a spectrum analysis for use in creating the events:
 *   Use ffmpeg with a size of (song length * 10). ex 205 seconds * 10 = 2050x512 as below.
 *    ex. ffmpeg -i "game.m4a" -lavfi showspectrumpic=s=2050x512:color=4:scale=lin:stop=8000 gameLd.png
 *
 * Ideas:
 *  https://learn.sparkfun.com/tutorials/prototype-wearable-led-dance-harness/all
 *  https://learn.sparkfun.com/tutorials/motion-controlled-wearable-led-dance-harness
 *  https://learn.sparkfun.com/tutorials/hackers-in-residence---sound-and-motion-reactivity-for-wearables
 * Radio: dw
 *  NRF24L01 https://www.instructables.com/Arduino-and-NRF24L01/
 *  RTF433 https://www.instructables.com/RF-315433-MHz-Transmitter-receiver-Module-and-Ardu/
 * JY-MCU Bluetooth module communication example for Arduino.
 *  Connect RX_PIN to TX pin of the module,
 *  Connect TX_PIN to RX pin of the module.
 * Set Bluetooth HC-05
 * 
 * 16k - For a bluetooth-enabled 300-addressableled-driving music-synchronized system
 */
#include "Fx.h"
#include "Track.h"
static FxState fxState = FxState_Default;

//////////////// FastLED Section ////////////////
#include <FastLED.h>
#define LED_PIN     3 // 3, 5 for some, should be 3 
#define NUM_LEDS    310
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
//////////////// FastLED Section ////////////////

//////////////// BlueTooth Section ////////////////
#include <SoftwareSerial.h>
const PROGMEM int RX_PIN = 5;
const PROGMEM int TX_PIN = 6;
const PROGMEM unsigned long BLUETOOTH_BAUD_RATE = 38400;
SoftwareSerial bluetooth(RX_PIN, TX_PIN);
//////////////// BlueTooth Section ////////////////

static uint8_t startIndex=0;
static bool animatePalette = false;
static float transitionMux = 0;
static unsigned long timeOffset = 0;
static unsigned long lastMatchedTimecode = 0;
static unsigned long lastTimeLed = 0;
static unsigned long GetTime() { return millis() - timeOffset; }

void Print(String str)
{
  Serial.print(str);
  bluetooth.print(str);
}

void Println(String str)
{
  Serial.println(str);
  bluetooth.println(str);
}

void trackStart()
{
  CRGB dk(0,0,0);
  fxState = FxState_PlayingTrack;
  fxController.currentPalette = CRGBPalette16(dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk);
  fxController.initialPalette = CRGBPalette16(dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk);
  fxController.paletteSpeed = 0;
  fxController.paletteDirection = 1;
  fxController.timedTransition = false;
  lastMatchedTimecode = 0;
  transitionMux = 0;
  timeOffset = (unsigned long)(millis() - (signed long)TRACK_START_DELAY);

  Print(F("Playing Track"));    
  Print(F(", Time Offset = "));
  Println(String(timeOffset));
}

void trackStop()
{
  fxState = FxState_Default;
  animatePalette = false;
  Print(F("Stopping Track"));
}

void FastLED_FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  uint8_t brightness = 255;    
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( fxController.currentPalette, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void FastLED_SetPalette()
{
  startIndex = startIndex + (fxController.paletteSpeed * fxController.paletteDirection);
  FastLED_FillLEDsFromPaletteColors( startIndex);
  FastLED.show();
}  

void setup() {
  Serial.begin(9600); //serial communication at 9600 bauds
  delay( 3000 ); // power-up safety delay  
  Serial.println(F("Startup Ok"));
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  FastLED_SetPalette();

  bluetooth.begin(BLUETOOTH_BAUD_RATE);
  Print(F("BT:"));
  Println(String(BLUETOOTH_BAUD_RATE));

  if (fxState == FxState_PlayingTrack)
    trackStart();
  else Println(F("Ready"));

  if (fxState == FxState_TestPattern)
  {
    Println(F("TestMode"));
    FxEventProcess(fx_palette_rgb);
    animatePalette = true;
  }
}

void FxEventPoll(unsigned long timecode)
{
  int match = GetCurrentTimeCodeMatch(timecode);
  int nextmatch = GetNextTimeCodeMatch(match);  
  unsigned long matchedTimecode = SongTrack_timecode(match);
  unsigned long nextMatchedTimecode = SongTrack_timecode(nextmatch);

  if (matchedTimecode > lastMatchedTimecode)
  {
    fxController.timedTransition = false;

    FxTrackSay(timecode, matchedTimecode, nextMatchedTimecode);
    Println(String((float)matchedTimecode/(float)1000.0f));
    Println(F(" : next @ "));
    Println(String((float)nextMatchedTimecode/(float)1000.0f));

    for (int i=0;i<numSongTracks;i++)
      if (SongTrack_timecode(i) == matchedTimecode)
        FxEventProcess(SongTrack_event(i));

    lastMatchedTimecode = timecode;
  }

  unsigned long totalSpan = nextMatchedTimecode - lastMatchedTimecode;  
  transitionMux = ((float)timecode - (float)lastMatchedTimecode ) / (float)totalSpan;

  if (fxController.timedTransition)
  {
    //Interpolate initial palette to next palette, based on transition (0 to 1)
    for (int i=0;i<16;i++)
    {
      CRGB rgb = LerpRGB(transitionMux,
        fxController.initialPalette[i][0],fxController.initialPalette[i][1],fxController.initialPalette[i][2],
        fxController.nextPalette[i][0],fxController.nextPalette[i][1],fxController.nextPalette[i][2]);        
      fxController.currentPalette[i] = rgb;
    } 
  }
}

void DirectEvent(int event)
{
  fxState = FxState_Default;
  fxController.timedTransition = false;
  Println(FxEventName(event));
  FxEventProcess(event);
}

void processInput(int data)
{
  switch (data)
  {
    case '?': 
      Println(F("? : Help Menu"));
      Println(F("+ : Rotate Pos"));
      Println(F("- : Rotate Neg"));
      Println(F("d : Track Start"));
      Println(F("s : Track Stop"));
      Println(F("0-9 : Color"));
      break;
    
    case ')':trackStart();break;
    case '(':trackStop();break;
    
    case '-':animatePalette=true;DirectEvent(fx_speed_neg);break;
    case '+':animatePalette=true;DirectEvent(fx_speed_pos);break;
    
    case 'q':animatePalette=true;DirectEvent(fx_speed_0);break;
    case 'w':animatePalette=true;DirectEvent(fx_speed_1);break;
    case 'e':animatePalette=true;DirectEvent(fx_speed_2);break;
    case 'r':animatePalette=true;DirectEvent(fx_speed_4);break;
    case 't':animatePalette=true;DirectEvent(fx_speed_6);break;
    case 'y':animatePalette=true;DirectEvent(fx_speed_8);break;
    case 'u':animatePalette=true;DirectEvent(fx_speed_16);break;
    case 'i':animatePalette=true;DirectEvent(fx_speed_32);break;
      
    case '0':animatePalette=false;DirectEvent(fx_palette_dark);FastLED_SetPalette();break;
    case '1':animatePalette=false;DirectEvent(fx_palette_white);FastLED_SetPalette();break;
    case '2':animatePalette=false;DirectEvent(fx_palette_red);FastLED_SetPalette();break;
    case '3':animatePalette=false;DirectEvent(fx_palette_yellow);FastLED_SetPalette();break;
    case '4':animatePalette=false;DirectEvent(fx_palette_green);FastLED_SetPalette();break;
    case '5':animatePalette=false;DirectEvent(fx_palette_cyan);FastLED_SetPalette();break;
    case '6':animatePalette=false;DirectEvent(fx_palette_blue);FastLED_SetPalette();break;
    case '7':animatePalette=false;DirectEvent(fx_palette_magenta);FastLED_SetPalette();break;
    case '8':animatePalette=false;DirectEvent(fx_palette_orange);FastLED_SetPalette();break;
    case '9':animatePalette=false;DirectEvent(fx_palette_rgb);FastLED_SetPalette();break;

    case 'z':animatePalette=false;DirectEvent(fx_palette_wr);FastLED_SetPalette();break;
    case 'x':animatePalette=false;DirectEvent(fx_palette_wy);FastLED_SetPalette();break;
    case 'c':animatePalette=false;DirectEvent(fx_palette_wg);FastLED_SetPalette();break;
    case 'v':animatePalette=false;DirectEvent(fx_palette_wc);FastLED_SetPalette();break;
    case 'b':animatePalette=false;DirectEvent(fx_palette_wb);FastLED_SetPalette();break;
    case 'n':animatePalette=false;DirectEvent(fx_palette_wm);FastLED_SetPalette();break;

    case 'a':animatePalette=false;DirectEvent(fx_palette_ry);FastLED_SetPalette();break;
    case 's':animatePalette=false;DirectEvent(fx_palette_rg);FastLED_SetPalette();break;
    case 'd':animatePalette=false;DirectEvent(fx_palette_rc);FastLED_SetPalette();break;
    case 'f':animatePalette=false;DirectEvent(fx_palette_rb);FastLED_SetPalette();break;
    case 'g':animatePalette=false;DirectEvent(fx_palette_rm);FastLED_SetPalette();break;
    case 'h':animatePalette=false;DirectEvent(fx_palette_yg);FastLED_SetPalette();break;
    case 'j':animatePalette=false;DirectEvent(fx_palette_yc);FastLED_SetPalette();break;
    case 'k':animatePalette=false;DirectEvent(fx_palette_yb);FastLED_SetPalette();break;
    case 'A':animatePalette=false;DirectEvent(fx_palette_ym);FastLED_SetPalette();break;
    case 'S':animatePalette=false;DirectEvent(fx_palette_gc);FastLED_SetPalette();break;
    case 'D':animatePalette=false;DirectEvent(fx_palette_gb);FastLED_SetPalette();break;
    case 'F':animatePalette=false;DirectEvent(fx_palette_gm);FastLED_SetPalette();break;
    case 'G':animatePalette=false;DirectEvent(fx_palette_cb);FastLED_SetPalette();break;
    case 'H':animatePalette=false;DirectEvent(fx_palette_cm);FastLED_SetPalette();break;
    case 'J':animatePalette=false;DirectEvent(fx_palette_bm);FastLED_SetPalette();break;

    case 0:
    case 10:
    case 13:
    case 225:break;
    default:
      Print(F("unk:"));
      Println(String(data));
      break; 
  }  
}

void loop()
{
  while (Serial.available())
    processInput(Serial.read()); 
  while (bluetooth.available())
    processInput(bluetooth.read());

  if (fxState == FxState_PlayingTrack)
    FxEventPoll(GetTime());    

  if (fxState == FxState_PlayingTrack || animatePalette)
  {
    unsigned long t =  millis();
    if (t - lastTimeLed > 45)//delay to let bluetooth get data
    {
      FastLED_SetPalette();
      lastTimeLed = t;
    }
  }  
}
