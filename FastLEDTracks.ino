/* FastLEDTracks : A time-track event system for FastLED
 *  Author: Layne Thomas
 *  This program can store and execute a millisecond-accurate lighting event system.
 *  Primary use is as a wearable costume synchronized to music for dance.
 *
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
#include <avr/pgmspace.h> 

#define LEAD                 1     // Set 1 for Dance lead, 0 for Dance follow

#define PLAY_AT_STARTUP      0     // Play right at startup?
#define BLUETOOTH_ENABLE     1     // Uses about 200 bytes
#define DEBUG_ENABLE         1     // Debug verbose mode
#define HEARTBEAT_OUTPUT     0     // Output device heartbeat 

#define DELAY_ENABLE         0
#define TRACK_START_DELAY    1800  // Delay time from start until track should truly 'start'

//////////////// FastLED Section ////////////////
#include <FastLED.h>
#define LED_PIN     3
#define NUM_LEDS    310
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 500
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
CRGBPalette16 initialPalette;
CRGBPalette16 nextPalette;
static uint8_t lerp(float mux, uint8_t a, uint8_t b) { return (uint8_t)(a * (1.0 - mux) + b * mux); }
static CRGB LerpRGB(float t, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) { return CRGB(lerp(t, r1, r2),lerp(t, g1, g2),lerp(t, b1, b2)); }
//////////////// FastLED Section ////////////////

static bool playing = false;
static bool timedTransition = false;
static int paletteSpeed = 1;
static float transitionMux = 0;
static unsigned long timeOffset = 0;
static unsigned long lastMatchedTimecode = 0;
static unsigned long lastTimeLed = 0;
static bool testMode = false;

//////////////// BlueTooth Section ////////////////
#if BLUETOOTH_ENABLE
 #include <SoftwareSerial.h>
 const PROGMEM int RX_PIN = 5;
 const PROGMEM int TX_PIN = 6;
 const PROGMEM unsigned long BLUETOOTH_BAUD_RATE = 38400;
 SoftwareSerial bluetooth(RX_PIN, TX_PIN);
#endif
//////////////// BlueTooth Section ////////////////

//List of possible Fx events
enum FxEvent
{
  fx_speed_0  = 0,
  fx_speed_1  = 1,
  fx_speed_2  = 2,
  fx_speed_3  = 3,
  fx_speed_4  = 4,
  fx_speed_5  = 5,
  fx_speed_6  = 6,
  fx_speed_7  = 7,
  fx_speed_8  = 8,
  fx_speed_9  = 9,
  fx_speed_10 = 10,
  fx_speed_11 = 11,
  fx_speed_12 = 12,
  fx_speed_13 = 13,
  fx_speed_14 = 14,
  fx_speed_15 = 15,
  fx_speed_16 = 16,
  fx_speed_17 = 17,
  fx_speed_18 = 18,
  fx_speed_19 = 19,

  fx_speed_pos = 20,
  fx_speed_neg = 21,

  fx_transition_fast = 30,
  fx_transition_timed = 31,  

  fx_palette_lead = 40,
  fx_palette_follow = 41,

  fx_palette_dark = 101,
  fx_palette_white = 102,
  fx_palette_red = 103,
  fx_palette_yellow = 104,
  fx_palette_green = 105,
  fx_palette_cyan = 106,
  fx_palette_blue = 107,
  fx_palette_magenta = 108,
  fx_palette_orange = 109,  
  
  fx_palette_dw = 119,  
  fx_palette_dr = 120,
  fx_palette_dy = 121,
  fx_palette_dg = 122,
  fx_palette_dc = 123,
  fx_palette_db = 124,
  fx_palette_dm = 125,
  fx_palette_wr = 130,
  fx_palette_wy = 131,
  fx_palette_wg = 132,
  fx_palette_wc = 133,
  fx_palette_wb = 134,
  fx_palette_wm = 135,
  fx_palette_ry = 141,
  fx_palette_rg = 142,
  fx_palette_rc = 143,
  fx_palette_rb = 144,
  fx_palette_rm = 145,
  fx_palette_yg = 152,
  fx_palette_yc = 153,
  fx_palette_yb = 154,
  fx_palette_ym = 155,
  fx_palette_gc = 163,
  fx_palette_gb = 164,
  fx_palette_gm = 165,
  fx_palette_cb = 174,
  fx_palette_cm = 175,
  fx_palette_bm = 185,

  fx_palette_wry = 201,
  fx_palette_wrg = 202,
  fx_palette_wrc = 203,
  fx_palette_wrb = 204,
  fx_palette_wrm = 205,
  fx_palette_wyg = 206,
  fx_palette_wyc = 207,
  fx_palette_wyb = 208,
  fx_palette_wym = 209,
  fx_palette_wgc = 210,
  fx_palette_wgb = 211,
  fx_palette_wgm = 212,
  fx_palette_wcb = 213,
  fx_palette_wcm = 214,
  fx_palette_wbm = 215,

  fx_palette_rgb = 230
};
struct Fx { unsigned long timecode; unsigned long event;   };

// Main Track set to 'The Game Has Changed'
#if LEAD
const unsigned long SongTrack[] PROGMEM = 
{
//Basic setup, fade in to blue
  1,fx_palette_dark,
  1,fx_speed_1,  
  1,fx_speed_pos,

//First snapin
  9633,fx_transition_fast,
  9633,fx_palette_lead,
  9633,fx_transition_timed,
  9633,fx_palette_dark,
//Second snapint  
  12033,fx_transition_fast,
  12033,fx_palette_follow,
  12033,fx_transition_timed,
  12033,fx_palette_dark,
//Third snapin  
  14366,fx_transition_fast,
  14366,fx_palette_lead,
  14366,fx_transition_timed,
  14366,fx_palette_dark,
//Fourth snapin
  16833,fx_transition_fast,
  16833,fx_palette_follow,
  16833,fx_transition_timed,
  16833,fx_palette_dark,

//March together
  19166,fx_transition_fast,
  19166,fx_palette_rb,
  19166,fx_transition_timed,
  19166,fx_palette_wrb,
  //19166,fx_speed_1,  

  //Coast
  24100,fx_transition_timed,
  24100,fx_palette_lead,
  26366,fx_transition_fast,
  26366,fx_palette_white,    
  26900,fx_palette_wb,

  //Build2
  28733,fx_palette_rb,

  //Coast2
  33633,fx_transition_timed,
  33633,fx_palette_cyan,
  35966,fx_transition_fast,    
  35966,fx_palette_white,    
  36466,fx_palette_wb,

  //Build3
  38400,fx_palette_rb,
  40766,fx_palette_wb,
  43166,fx_palette_rb,
  45566,fx_palette_wb,

  //Breakthrough to the G
  48100,fx_transition_timed,
  48100,fx_speed_0,  
  48100,fx_palette_magenta,
  52766,fx_palette_cyan,
  59933,fx_palette_yellow,
  62366,fx_palette_orange,
  
  //coasting orange 
  69566,fx_palette_orange,
  69566,fx_speed_1,
  69566,fx_transition_fast,

//placeholder
  80500, fx_palette_dark,

  //the end
  205000, fx_palette_dark
};
#else //Follow
const PROGMEM Fx SongTrack[] = { };
#endif
const PROGMEM int numSongTracks = sizeof(SongTrack)/(sizeof(unsigned long)*2);

//Compacted utility functions
static unsigned long SongTrack_timecode(int i) { return pgm_read_dword(&(SongTrack[i*2+0])); } 
static unsigned long SongTrack_event(int i) {  return pgm_read_dword(&(SongTrack[i*2+1])); }
static unsigned long GetTime() { return millis() - timeOffset; }
static void ProcessSerialInput() { while (Serial.available()) { int data = Serial.read(); Serial.print(data); processInput(data); } }
int GetNextTimeCodeMatch(int currentMatch) { unsigned long tc = SongTrack_timecode(currentMatch); for (int i=0;i<numSongTracks;i++) if (SongTrack_timecode(i) > tc) return i; return 0; }
int GetCurrentTimeCodeMatch(unsigned long timecode) { int match = 0; for (int i=0;i<numSongTracks;i++) { if (SongTrack_timecode(i) <= timecode) match = i; } return match; }

void FxEventTransition() { 
  initialPalette = currentPalette; timedTransition = true; 
}

#if DEBUG_ENABLE
String FxEventName(int event)
{  
  switch(event)
  {
    case fx_speed_pos: return F("pos");break;
    case fx_speed_neg: return F("neg");break;
    case fx_speed_0: return F("x0");break;
    case fx_speed_1: return F("x1");break;
    case fx_speed_2: return F("x2");break;
    case fx_speed_3: return F("x3");break;
    case fx_speed_4: return F("x4");break;
    case fx_speed_5: return F("x5");break;
    case fx_speed_6: return F("x6");break;
    case fx_speed_7: return F("x7");break;
    case fx_speed_8: return F("x8");break;
    case fx_speed_9: return F("x9");break;
    case fx_speed_10: return F("x10");break;
    case fx_speed_11: return F("x11");break;
    case fx_speed_12: return F("x12");break;
    case fx_speed_13: return F("x13");break;
    case fx_speed_14: return F("x14");break;
    case fx_speed_15: return F("x15");break;
    case fx_speed_16: return F("x16");break;
    case fx_speed_17: return F("x17");break;
    case fx_speed_18: return F("x18");break;
    case fx_speed_19: return F("x19");break;
    
    case fx_transition_timed: return F("timed");break;
    case fx_transition_fast: return F("fast");break;
    case fx_palette_lead: return F("lead");break;    
    case fx_palette_follow: return F("follow");break;       

    case fx_palette_dark: return F("dk");break;
    case fx_palette_white: return F("w");break;
    case fx_palette_red: return F("r");break;
    case fx_palette_yellow: return F("y");break;
    case fx_palette_green: return F("g");break;
    case fx_palette_cyan: return F("c");break;
    case fx_palette_blue: return F("b");break;
    case fx_palette_magenta: return F("m");break;
    case fx_palette_orange: return F("o");break;
   
    case fx_palette_dw: return F("dw");break;
    case fx_palette_dr: return F("dr");break;
    case fx_palette_dy: return F("dy");break;
    case fx_palette_dg: return F("dg");break;
    case fx_palette_dc: return F("dc");break;
    case fx_palette_db: return F("db");break;
    case fx_palette_dm: return F("dm");break;
    case fx_palette_wr: return F("wr");break;
    case fx_palette_wy: return F("wy");break;
    case fx_palette_wg: return F("wg");break;
    case fx_palette_wc: return F("wc");break;
    case fx_palette_wb: return F("wb");break;
    case fx_palette_wm: return F("wm");break;
    case fx_palette_ry: return F("ry");break;
    case fx_palette_rg: return F("rg");break;
    case fx_palette_rc: return F("rc");break;
    case fx_palette_rb: return F("rb");break;
    case fx_palette_rm: return F("rm");break;
    case fx_palette_yg: return F("yg");break;
    case fx_palette_yc: return F("yc");break;
    case fx_palette_yb: return F("yb");break;
    case fx_palette_ym: return F("ym");break;
    case fx_palette_gc: return F("gc");break;
    case fx_palette_gb: return F("gb");break;
    case fx_palette_gm: return F("gm");break;
    case fx_palette_cb: return F("cb");break;
    case fx_palette_cm: return F("cm");break;
    case fx_palette_bm: return F("bm");break;

    case fx_palette_wry:return F("wry");break;
    case fx_palette_wrg:return F("wrg");break;
    case fx_palette_wrc:return F("wrc");break;
    case fx_palette_wrb:return F("wrb");break;
    case fx_palette_wrm:return F("wrm");break;
    case fx_palette_wyg:return F("wyg");break;
    case fx_palette_wyc:return F("wyc");break;
    case fx_palette_wyb:return F("wyb");break;
    case fx_palette_wym:return F("wym");break;
    case fx_palette_wgc:return F("wgc");break;
    case fx_palette_wgb:return F("wgb");break;
    case fx_palette_wgm:return F("wgm");break;
    case fx_palette_wcb:return F("wcb");break;
    case fx_palette_wcm:return F("wcm");break;
    case fx_palette_wbm:return F("wbm");break;

    case fx_palette_rgb: return F("rgb");break;
  }
}
#endif

#define DARK    0x00,0x00,0x00
#define WHITE   0xFF,0xFF,0xFF
#define RED     0xFF,0x00,0x00
#define YELLOW  0xFF,0xFF,0x00
#define GREEN   0x00,0xFF,0x00
#define CYAN    0x00,0xFF,0xFF
#define BLUE    0x00,0x00,0xFF
#define MAGENTA 0xFF,0x00,0xFF
#define ORANGE  0xFF,0x7F,0x00
void FxEventProcess(int state)
{
  switch (state)
  {
    case fx_speed_0:
    case fx_speed_1:
    case fx_speed_2:
    case fx_speed_3:
    case fx_speed_4:
    case fx_speed_5:
    case fx_speed_6:
    case fx_speed_7:
    case fx_speed_8:
    case fx_speed_9:
    case fx_speed_10:
    case fx_speed_11:
    case fx_speed_12:
    case fx_speed_13:
    case fx_speed_14:
    case fx_speed_15:
    case fx_speed_16:
    case fx_speed_17:
    case fx_speed_18:
    case fx_speed_19:
      paletteSpeed = state;
      break;

    case fx_speed_pos:paletteSpeed = abs(paletteSpeed);break;
    case fx_speed_neg:paletteSpeed = -abs(paletteSpeed);break;

    case fx_transition_fast:timedTransition = false;break;
    case fx_transition_timed:timedTransition = true;break;

    case fx_palette_lead:CreateSingleBand(BLUE);break;
    case fx_palette_follow:CreateSingleBand(RED);break;    
   
    case fx_palette_dark:CreateSingleBand(DARK);break;
    case fx_palette_white:CreateSingleBand(WHITE);break;
    case fx_palette_red:CreateSingleBand(RED);break;
    case fx_palette_yellow:CreateSingleBand(YELLOW);break;
    case fx_palette_green:CreateSingleBand(GREEN);break;
    case fx_palette_cyan:CreateSingleBand(CYAN);break;
    case fx_palette_blue:CreateSingleBand(BLUE);break;
    case fx_palette_magenta:CreateSingleBand(MAGENTA);break;
    case fx_palette_orange:CreateSingleBand(ORANGE);break;
    
    case fx_palette_dr: CreateDoubleBand(DARK, RED); break;
    case fx_palette_dy: CreateDoubleBand(DARK, YELLOW); break;
    case fx_palette_dg: CreateDoubleBand(DARK, GREEN); break;
    case fx_palette_dc: CreateDoubleBand(DARK, CYAN); break;
    case fx_palette_db: CreateDoubleBand(DARK, BLUE); break;
    case fx_palette_dm: CreateDoubleBand(DARK, MAGENTA); break;
    case fx_palette_wr: CreateDoubleBand(WHITE, RED); break;
    case fx_palette_wy: CreateDoubleBand(WHITE, YELLOW); break;
    case fx_palette_wg: CreateDoubleBand(WHITE, GREEN); break;
    case fx_palette_wc: CreateDoubleBand(WHITE, CYAN); break;
    case fx_palette_wb: CreateDoubleBand(WHITE, BLUE); break;
    case fx_palette_wm: CreateDoubleBand(WHITE, MAGENTA); break;
    case fx_palette_ry: CreateDoubleBand(RED, YELLOW); break;
    case fx_palette_rg: CreateDoubleBand(RED, GREEN); break;
    case fx_palette_rc: CreateDoubleBand(RED, CYAN); break;
    case fx_palette_rb: CreateDoubleBand(RED, BLUE); break;
    case fx_palette_rm: CreateDoubleBand(RED, MAGENTA); break;
    case fx_palette_yg: CreateDoubleBand(YELLOW, GREEN); break;
    case fx_palette_yc: CreateDoubleBand(YELLOW, CYAN); break;
    case fx_palette_yb: CreateDoubleBand(YELLOW, BLUE); break;
    case fx_palette_ym: CreateDoubleBand(YELLOW, MAGENTA); break;
    case fx_palette_gc: CreateDoubleBand(GREEN, CYAN); break;
    case fx_palette_gb: CreateDoubleBand(GREEN, BLUE); break;
    case fx_palette_gm: CreateDoubleBand(GREEN, MAGENTA); break;
    case fx_palette_cb: CreateDoubleBand(CYAN, BLUE); break;
    case fx_palette_cm: CreateDoubleBand(CYAN, MAGENTA); break;
    case fx_palette_bm: CreateDoubleBand(BLUE, MAGENTA); break;

    case fx_palette_wry: CreateTripleBand(WHITE, RED, YELLOW); break;
    case fx_palette_wrg: CreateTripleBand(WHITE, RED, GREEN); break;
    case fx_palette_wrc: CreateTripleBand(WHITE, RED, CYAN); break;
    case fx_palette_wrb: CreateTripleBand(WHITE, RED, BLUE); break;
    case fx_palette_wrm: CreateTripleBand(WHITE, RED, MAGENTA); break;
    case fx_palette_wyg: CreateTripleBand(WHITE, YELLOW, GREEN); break;
    case fx_palette_wyc: CreateTripleBand(WHITE, YELLOW, CYAN); break;
    case fx_palette_wyb: CreateTripleBand(WHITE, YELLOW, BLUE); break;
    case fx_palette_wym: CreateTripleBand(WHITE, YELLOW, MAGENTA); break;
    case fx_palette_wgc: CreateTripleBand(WHITE, GREEN, CYAN); break;
    case fx_palette_wgb: CreateTripleBand(WHITE, GREEN, BLUE); break;
    case fx_palette_wgm: CreateTripleBand(WHITE, GREEN, MAGENTA); break;
    case fx_palette_wcb: CreateTripleBand(WHITE, CYAN, BLUE); break;
    case fx_palette_wcm: CreateTripleBand(WHITE, CYAN, MAGENTA); break;
    case fx_palette_wbm: CreateTripleBand(WHITE, BLUE, MAGENTA); break;

    case fx_palette_rgb: CreateTripleBand(RED, GREEN, BLUE);break;
  }
}

void trackSetup()
{
  CRGB dk(0,0,0);
  currentPalette = CRGBPalette16(dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk);
  initialPalette = CRGBPalette16(dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk,dk);
  paletteSpeed = 0;
  lastMatchedTimecode = 0;
  timedTransition = false;
  transitionMux = 0;
  timeOffset = (unsigned long)(millis() - (signed long)TRACK_START_DELAY);
#if DEBUG_ENABLE
  Serial.print(F("Time Offset = "));
  Serial.println(timeOffset);
#endif
}

void setup() {
  Serial.begin(9600); //serial communication at 9600 bauds

  delay( 3000 ); // power-up safety delay  
  Serial.println(F("Startup Ok"));
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
/*
  trackSetup();
  timeOffset = 0;
  timeDelay = GetTime() + timeDelay;  
  Serial.print(F("TimeDelay = "));
  Serial.println(timeDelay);
  */

#if BLUETOOTH_ENABLE
   bluetooth.begin(BLUETOOTH_BAUD_RATE);
   Serial.print(F("BT Baud Rate:"));
   Serial.println(BLUETOOTH_BAUD_RATE);
   bluetooth.println(F("BT Ok"));
#endif

#if DEBUG_ENABLE
  if (playing) Serial.println(F("Playing"));
  else Serial.println(F("Ready"));
#endif

#if testMode
  Serial.println(F("TestMode"));
#endif  

#if PLAY_AT_STARTUP
  playing = true;
#endif
}


#if DEBUG_ENABLE
void FxEventsSay(unsigned long timecode, unsigned long matchedTimecode,unsigned long nextMatchedTimecode)
{
    float tc = (float)matchedTimecode / (float)1000.0f;
    Serial.print(tc);
    Serial.print(F(" :"));
    for (int i=0;i<numSongTracks;i++)
    {
      if (SongTrack_timecode(i) == matchedTimecode)
      {
        Serial.print(F(" "));
        Serial.print(FxEventName(SongTrack_event(i)));
      }
    }

    Serial.print(F(", next"));
/*    
    Serial.print(F(" = "));
    for (int i=0;i<numSongTracks;i++)
      if (activeSongTracks[i].timecode == nextMatchedTimecode)
          FxEventSay(activeSongTracks[i].state);        
*/  
    float timeUntil = (float)(nextMatchedTimecode - (float)timecode) / 1000.0f;
    Serial.print(F(" in "));
    Serial.print(timeUntil);
    Serial.print(F("s"));
    Serial.println();
}
#endif

void FxEventPoll(unsigned long timecode)
{
  if (!playing)
    return;
 
  int match = GetCurrentTimeCodeMatch(timecode);
  int nextmatch = GetNextTimeCodeMatch(match);  
  unsigned long matchedTimecode = SongTrack_timecode(match);
  unsigned long nextMatchedTimecode = SongTrack_timecode(nextmatch);

  if (matchedTimecode > lastMatchedTimecode)
  {
    timedTransition = false;

#if DEBUG_ENABLE
    FxEventsSay(timecode, matchedTimecode, nextMatchedTimecode);
#endif    
#if BLUETOOTH_ENABLE
    bluetooth.print((float)matchedTimecode/(float)1000.0f);
    bluetooth.print(F(" : next @ "));
    bluetooth.println((float)nextMatchedTimecode/(float)1000.0f);
#endif

    for (int i=0;i<numSongTracks;i++)
      if (SongTrack_timecode(i) == matchedTimecode)
        FxEventProcess(SongTrack_event(i));

    lastMatchedTimecode = timecode;
  }

  unsigned long totalSpan = nextMatchedTimecode - lastMatchedTimecode;  
  transitionMux = ((float)timecode - (float)lastMatchedTimecode ) / (float)totalSpan;

  if (timedTransition)
  {
    //Interpolate initial palette to next palette, based on transition (0 to 1)
    for (int i=0;i<16;i++)
    {
      CRGB rgb = LerpRGB(transitionMux,
        initialPalette[i][0],initialPalette[i][1],initialPalette[i][2],
        nextPalette[i][0],nextPalette[i][1],nextPalette[i][2]);        
      currentPalette[i] = rgb;
    } 
  }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, LINEARBLEND);
        colorIndex += 3;
    }
}

void CreateSingleBand(uint8_t r, uint8_t g, uint8_t b) { return CreateQuadBand(r,g,b,r,g,b,r,g,b,r,g,b); }
void CreateDoubleBand(uint8_t r1, uint8_t g1, uint8_t b1,uint8_t r2, uint8_t g2, uint8_t b2) { return CreateQuadBand(r1,g1,b1,r2,g2,b2,r1,g1,b1,r2,g2,b2); }
void CreateTripleBand(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t r3, uint8_t g3, uint8_t b3) { return CreateQuadBand(r1,g1,b1,r2,g2,b2,r3,g3,b3,0,0,0); }
void CreateQuadBand(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t r3, uint8_t g3, uint8_t b3, uint8_t r4, uint8_t g4, uint8_t b4)
{
    if (timedTransition)
    {
      nextPalette = CRGBPalette16(CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4));
      FxEventTransition();
    }
    else  
    {
      currentPalette = CRGBPalette16(CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                    CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                    CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                    CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4));
    }
}

void Print(String str)
{
#if DEBUG_ENABLE
        Serial.println(str);
#endif        
#if BLUETOOTH_ENABLE
        bluetooth.println(str);
#endif
}

void ForcePalette()
{
    Print(F("Pal"));    
    FillLEDsFromPaletteColors(0);
    FastLED.show(); 
    
    testMode = true;
    playing  = true;
}

void processInput(int data)
{
    switch (data)
    {
      case '0': timedTransition = false;FxEventProcess(fx_palette_dark);Print(F("DARK"));ForcePalette();break;
      case '1': timedTransition = false;FxEventProcess(fx_palette_white);Print(F("WHITE"));ForcePalette();break;
      case '2': timedTransition = false;FxEventProcess(fx_palette_red);Print(F("RED"));ForcePalette();break;
      case '3': timedTransition = false;FxEventProcess(fx_palette_yellow);Print(F("YELLOW"));ForcePalette();break;
      case '4': timedTransition = false;FxEventProcess(fx_palette_green);Print(F("GREEN"));ForcePalette();break;
      case '5': timedTransition = false;FxEventProcess(fx_palette_cyan);Print(F("CYAN"));ForcePalette();break;
      case '6': timedTransition = false;FxEventProcess(fx_palette_blue);Print(F("BLUE"));ForcePalette();break;
      case '7': timedTransition = false;FxEventProcess(fx_palette_magenta);Print(F("MAGENTA"));ForcePalette();break;
      case '8': timedTransition = false;FxEventProcess(fx_palette_orange);Print(F("ORANGE"));ForcePalette();break;
      case 't':
        Print(F("Test"));
        break;
      case 'm': 
        if (playing) Print(F("b m s : Playing"));
        else Print(F("b m s : Ready"));
        break;
      case 'b': 
        Print(F("Begin Track"));
        trackSetup();
        testMode = 0;
        playing = true;
        break;
      case 's': 
        Print(F("Stopping Track"));
        playing = false;
        break;
      case 10:
      case 13:
      case 225:break;
      default:
        Print(F("unk:"));
        break; 
    }  
}



void loop()
{
#if HEARTBEAT_OUTPUT
  Serial.println(millis());
#endif

#if BLUETOOTH_ENABLE
  while (bluetooth.available()) {
    char data = bluetooth.read();
    if (data != 10 && data != 13 && data != 225)
    {
      Serial.print(F("RCV:"));
      Serial.println(data);
      processInput(data);
    }
  }  
#endif
  ProcessSerialInput();

  if (!testMode)
    FxEventPoll(GetTime());    

  if (playing)
  {
    unsigned long t =  millis();
    if (t - lastTimeLed > 45)//delay to let bluetooth get data
    {
      static uint8_t startIndex = startIndex + (paletteSpeed);
      FillLEDsFromPaletteColors( startIndex);
      FastLED.show();
      //FastLED.delay(1000 / UPDATES_PER_SECOND);
      lastTimeLed = t;
    }
    if (testMode) playing = false;
  }
}
