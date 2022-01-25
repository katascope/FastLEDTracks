/* Time-Track system for FastLED Addressable LED lighting rigs, by Layne Thomas 1-21-2022-11:03
 *  This program can store an execute a millisecond-accurate lighting event system.
 *  
 *  See TracksLead and TracksFollow 
 *  To create a spectrum analysis for use in creating the events:
 *  Use ffmpeg with a size of (song length * 10). ex 205 seconds * 10 = 2050x512 as below.
 *   ex. ffmpeg -i "game.m4a" -lavfi showspectrumpic=s=2050x512:color=4:scale=lin:stop=8000 gameLd.png
 *   
 *  LEAD : 1 for LEAD, 0 for follow
 *  DEBUG : Enabling this will print debug information
 *    
 *  Design Criteria:
 *   Tasteful : Use complex colors, avoid 'rgb rainbow land'
 *   Smooth : Fading color transitions over palette rotations
 *   Fade-To interpolated timing so endings match startings
 *   
 *  Software criteria:
 *   
 *  
 *  Hardware criteria:
 *   Minimal : The device must optimize memory usage.
 *   Cheap : The device must be low cost and easy to produce.
 *   Compatible : The device must maximize use of standards.
 *   Portable : The device must be human wearable and untethered.
 *   Stable : The device must withstand repeated heavy use dancing.
 *   Subtle : Indirect lighting for led's, hazed frosted fogged.
 *   Simple : The device must be extremely easy to use once setup.
 *   Recoverable : The device should be able to recover from mistimings.
 *   Lead-Follow : The follow device will synchronize to the lead device.
 *   Audioreactive : The device must represent the music.
 *   
 *  Standards:
 *   Arduino Nano : Standard microcontroller kit
 *   WS2811 : Addressable LED wire : Standard 3-wire connectors
 *   BTF-Lighting Connectors : Addressable 3-wire LED wire connectors
 *    Also used for power connector into arduino.
 *   XT-60 Battery connector with D-Sun BEC Voltage convert (3-cell 12v down to 5v)
 *   or 18650 Battery Shield case using 5-V output
 *   Thingiverse Case & plans
 *   ffmpeg for Spectrum Analysis
 *   
 *  Tradeoffs:
 *   Audio track setup does not have to be easy or quick.
 *   Different arduinos can be used.
 *   Different forms of power/battery can be used.
 *   
 *   
*/

#define LEAD 1       // Set 1 for Dance lead, 0 for Dance follow
int timeDelay = -200;//Bootup time less for later
#define DEBUG 1

#include <FastLED.h>
#define LED_PIN     5

#if DEBUG
#define NUM_LEDS    10
#else
#define NUM_LEDS    320
#endif

#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
CRGBPalette16 initialPalette;
CRGBPalette16 nextPalette;
TBlendType    currentBlending = LINEARBLEND;
int paletteSpeed = 1;
int paletteDirection = 1;
unsigned long bootupTime = 0;
unsigned long lastMatchedTimecode = 0;
bool timedTransition = false;
CRGB transitionTo;
float transition = 0;
float fastTransition = true;

//List of possible Fx events
enum FxEvent
{
  fx_unknown = 0,

  fx_speed_pos = 1,
  fx_speed_neg = 2,
  fx_speed_0 = 10,
  fx_speed_1 = 11,
  fx_speed_2 = 12,
  fx_speed_3 = 13,
  fx_speed_4 = 14,
  fx_speed_8 = 15,
  fx_speed_16 = 16,

  fx_transition_fast = 20,
  fx_transition_timed = 21,  

  fx_palette_lead = 30,
  fx_palette_follow = 31,

  fx_palette_black = 101,
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
struct Fx { unsigned long timecode; FxEvent event;   };

// Main Track set to 'The Game Has Changed'

#if LEAD
//Leads track
Fx TracksLead[] = 
{
  //Basic setup, fade in to blue
  {1,fx_palette_black},
  {1,fx_speed_0},  
  {1,fx_speed_pos},
  {1,fx_transition_timed},
  {1,fx_palette_blue},

  //Introduction
  {9633,fx_transition_fast},
  {9633,fx_palette_red},
  
  {12033,fx_palette_blue},
  {14366,fx_palette_red},
  {16833,fx_palette_blue},

  //First build-up
  {19166,fx_palette_rb},
  {19166,fx_speed_1},  

  //Coast
  {24100,fx_transition_timed},
  {24100,fx_palette_blue},
  {26366,fx_transition_fast},
  {26366,fx_palette_white},    
  {26900,fx_palette_wb},

  //Build2
  {28733,fx_palette_rb},

  //Coast2
  {33633,fx_transition_timed},
  {33633,fx_palette_blue},
  {35966,fx_transition_fast},    
  {35966,fx_palette_white},    
  {36466,fx_palette_wb},

  //Build3
  {38400,fx_palette_rb},
  {40766,fx_palette_wb},
  {43166,fx_palette_rb},
  {45566,fx_palette_wb},

  //Breakthrough to the G
  {48100,fx_transition_timed},
  {48100,fx_speed_0},  
  {48100,fx_palette_magenta},
  {52766,fx_palette_cyan},
  {59933,fx_palette_yellow},
  {62366,fx_palette_orange},
  
  //coasting orange 
  {69566,fx_palette_orange},
  {69566,fx_speed_1},
  {69566,fx_transition_fast},

//placeholder
  {80500, fx_palette_black},

  //the end
  {205000, fx_palette_black},
};
const int numTracks = sizeof(TracksLead)/sizeof(Fx);
Fx *activeTracks = &TracksLead[0];
#else
//Follows track
Fx TracksFollow[] = 
{
  //Basic setup, fade in to blue
  {1,fx_palette_black},
  {1,fx_speed_0},  
  {1,fx_speed_pos},
  {1,fx_transition_timed},
  {1,fx_palette_blue},

  //Introduction
  {9633,fx_transition_fast},
  {9633,fx_palette_red},
  
  {12033,fx_palette_blue},
  {14366,fx_palette_red},
  {16833,fx_palette_blue},

  //First build-up
  {19166,fx_palette_rb},
  {19166,fx_speed_1},  

  //Coast
  {24100,fx_transition_timed},
  {24100,fx_palette_blue},
  {26366,fx_transition_fast},
  {26366,fx_palette_white},    
  {26900,fx_palette_wb},

  //Build2
  {28733,fx_palette_rb},

  //Coast2
  {33633,fx_transition_timed},
  {33633,fx_palette_blue},
  {35966,fx_transition_fast},    
  {35966,fx_palette_white},    
  {36466,fx_palette_wb},

  //Build3
  {38400,fx_palette_rb},
  {40766,fx_palette_wb},
  {43166,fx_palette_rb},
  {45566,fx_palette_wb},

  //Breakthrough to the G
  {48100,fx_transition_timed},
  {48100,fx_palette_magenta},
  {52766,fx_palette_cyan},
  {59933,fx_palette_yellow},
  {62366,fx_palette_orange},
  
  //coasting orange 
  {69566,fx_palette_orange},
  {69566,fx_speed_1},
  {69566,fx_transition_fast},

//placeholder
  {80500, fx_palette_black},

  //the end
  {205000, fx_palette_black},
};
const int numTracks = sizeof(TracksFollow)/sizeof(Fx);
Fx *activeTracks = &TracksFollow[0];
#endif

#if DEBUG
void FxEventSay(int state)
{  
  Serial.print(" ");
  switch(state)
  {
    case fx_unknown: Serial.print("unknown");break;
    
    case fx_speed_pos: Serial.print("pos");break;
    case fx_speed_neg: Serial.print("neg");break;
    case fx_speed_0: Serial.print("x0");break;
    case fx_speed_1: Serial.print("x1");break;
    case fx_speed_2: Serial.print("x2");break;
    case fx_speed_3: Serial.print("x3");break;
    case fx_speed_4: Serial.print("x4");break;
    case fx_speed_8: Serial.print("x8");break;
    case fx_speed_16: Serial.print("x16");break;

    case fx_transition_timed: Serial.print("transition-timed");break;
    case fx_transition_fast: Serial.print("transition-fast");break;

    case fx_palette_lead: Serial.print("lead");break;    
    case fx_palette_follow: Serial.print("follow");break;       

    case fx_palette_black: Serial.print("black");break;
    case fx_palette_white: Serial.print("white");break;
    case fx_palette_red: Serial.print("red");break;
    case fx_palette_yellow: Serial.print("yellow");break;
    case fx_palette_green: Serial.print("green");break;
    case fx_palette_cyan: Serial.print("cyan");break;
    case fx_palette_blue: Serial.print("blue");break;
    case fx_palette_magenta: Serial.print("magenta");break;
    case fx_palette_orange: Serial.print("orange");break;
   
    case fx_palette_dw: Serial.print("dark-white");break;
    case fx_palette_dr: Serial.print("dark-red");break;
    case fx_palette_dy: Serial.print("dark-yellow");break;
    case fx_palette_dg: Serial.print("dark-green");break;
    case fx_palette_dc: Serial.print("dark-cyan");break;
    case fx_palette_db: Serial.print("dark-blue");break;
    case fx_palette_dm: Serial.print("dark-magenta");break;
    case fx_palette_wr: Serial.print("white-red");break;
    case fx_palette_wy: Serial.print("white-yellow");break;
    case fx_palette_wg: Serial.print("white-green");break;
    case fx_palette_wc: Serial.print("white-cyan");break;
    case fx_palette_wb: Serial.print("white-blue");break;
    case fx_palette_wm: Serial.print("white-magenta");break;
    case fx_palette_ry: Serial.print("red-yellow");break;
    case fx_palette_rg: Serial.print("red-green");break;
    case fx_palette_rc: Serial.print("red-cyan");break;
    case fx_palette_rb: Serial.print("red-blue");break;
    case fx_palette_rm: Serial.print("red-magenta");break;
    case fx_palette_yg: Serial.print("yellow-green");break;
    case fx_palette_yc: Serial.print("yellow-cyan");break;
    case fx_palette_yb: Serial.print("yellow-blue");break;
    case fx_palette_ym: Serial.print("yellow-magenta");break;
    case fx_palette_gc: Serial.print("green-cyan");break;
    case fx_palette_gb: Serial.print("green-blue");break;
    case fx_palette_gm: Serial.print("green-magenta");break;
    case fx_palette_cb: Serial.print("cyan-blue");break;
    case fx_palette_cm: Serial.print("cyan-magenta");break;
    case fx_palette_bm: Serial.print("blue-magenta");break;
    
    case fx_palette_rgb: Serial.print("red-green-blue");break;
  }
}
#endif

void FxEventTransition()
{
  initialPalette = currentPalette;
  timedTransition = true;
}

void FxEventProcess(int state)
{
  switch (state)
  {
//#if DEBUG    
    case fx_unknown: Serial.println("ERROR : Unknown Found!");break;
//#endif
    
    case fx_speed_pos:paletteDirection = 1;break;
    case fx_speed_neg:paletteDirection = -1;break;
    case fx_speed_0:paletteSpeed = 0;break;
    case fx_speed_1:paletteSpeed = 1;break;
    case fx_speed_2:paletteSpeed = 2;break;
    case fx_speed_3:paletteSpeed = 3;break;
    case fx_speed_4:paletteSpeed = 4;break;
    case fx_speed_16:paletteSpeed = 8;break;

    case fx_transition_fast:fastTransition = true;break;
    case fx_transition_timed:fastTransition = false;break;

    case fx_palette_lead:CreateSingleBand(0,0,255);break;
    case fx_palette_follow:CreateSingleBand(255,0,0);break;    
   
    case fx_palette_black:CreateSingleBand(0,0,0);break;
    case fx_palette_white:CreateSingleBand(255,255,255);break;
    case fx_palette_red:CreateSingleBand(255,0,0);break;
    case fx_palette_yellow:CreateSingleBand(255,255,0);break;
    case fx_palette_green:CreateSingleBand(0,255,0);break;
    case fx_palette_cyan:CreateSingleBand(0,255,255);break;
    case fx_palette_blue:CreateSingleBand(0,0,255);break;
    case fx_palette_magenta:CreateSingleBand(255,0,255);break;
    case fx_palette_orange:CreateSingleBand(255,127,0);break;
        
    case fx_palette_dw: CreateDoubleBand(0,0,0, 255, 255, 255); break;
    case fx_palette_dr: CreateDoubleBand(0,0,0, 255, 0, 0); break;
    case fx_palette_dy: CreateDoubleBand(0,0,0, 255, 255, 0); break;
    case fx_palette_dg: CreateDoubleBand(0,0,0, 0, 255, 0); break;
    case fx_palette_dc: CreateDoubleBand(0,0,0, 0, 255, 255); break;
    case fx_palette_db: CreateDoubleBand(0,0,0, 0, 0, 255); break;
    case fx_palette_dm: CreateDoubleBand(0,0,0, 255, 0, 255); break;
    case fx_palette_wr: CreateDoubleBand(255,255,255, 255, 0, 0); break;
    case fx_palette_wy: CreateDoubleBand(255,255,255, 255, 255, 0); break;
    case fx_palette_wg: CreateDoubleBand(255,255,255, 0, 255, 0); break;
    case fx_palette_wc: CreateDoubleBand(255,255,255, 0, 255, 255); break;
    case fx_palette_wb: CreateDoubleBand(255,255,255, 0, 0, 255); break;
    case fx_palette_wm: CreateDoubleBand(255,255,255, 255, 0, 255); break;
    case fx_palette_ry: CreateDoubleBand(255,0, 0, 255, 255, 0); break;
    case fx_palette_rg: CreateDoubleBand(255,0, 0, 0, 255, 0); break;
    case fx_palette_rc: CreateDoubleBand(255,0, 0, 0, 255, 255); break;
    case fx_palette_rb: CreateDoubleBand(255,0, 0, 0, 0, 255); break;
    case fx_palette_rm: CreateDoubleBand(255,0, 0, 255, 0, 255); break;
    case fx_palette_yg: CreateDoubleBand(255,255, 0, 0, 255, 0); break;
    case fx_palette_yc: CreateDoubleBand(255,255, 0, 0, 255, 255); break;
    case fx_palette_yb: CreateDoubleBand(255,255, 0, 0, 0, 255); break;
    case fx_palette_ym: CreateDoubleBand(255,255, 0, 255,0, 255); break;
    case fx_palette_gc: CreateDoubleBand(0,255,0, 0, 255, 255); break;
    case fx_palette_gb: CreateDoubleBand(0,255,0, 0, 0, 255); break;
    case fx_palette_gm: CreateDoubleBand(0,255,0, 255, 0, 255); break;
    case fx_palette_cb: CreateDoubleBand(0,255,255, 0, 0, 255); break;
    case fx_palette_cm: CreateDoubleBand(0,255,255, 255, 0, 255); break;
    case fx_palette_bm: CreateDoubleBand(255,0,255, 0, 0, 255); break;

    case fx_palette_wry: CreateTripleBand(255,255,255, 255,0, 0, 255, 255, 0); break;
    case fx_palette_wrg: CreateTripleBand(255,255,255,255,0, 0, 0, 255, 0); break;
    case fx_palette_wrc: CreateTripleBand(255,255,255,255,0, 0, 0, 255, 255); break;
    case fx_palette_wrb: CreateTripleBand(255,255,255,255,0, 0, 0, 0, 255); break;
    case fx_palette_wrm: CreateTripleBand(255,255,255,255,0, 0, 255, 0, 255); break;
    case fx_palette_wyg: CreateTripleBand(255,255,255,255,255, 0, 0, 255, 0); break;
    case fx_palette_wyc: CreateTripleBand(255,255,255,255,255, 0, 0, 255, 255); break;
    case fx_palette_wyb: CreateTripleBand(255,255,255,255,255, 0, 0, 0, 255); break;
    case fx_palette_wym: CreateTripleBand(255,255,255,255,255, 0, 255,0, 255); break;
    case fx_palette_wgc: CreateTripleBand(255,255,255,0,255,0, 0, 255, 255); break;
    case fx_palette_wgb: CreateTripleBand(255,255,255,0,255,0, 0, 0, 255); break;
    case fx_palette_wgm: CreateTripleBand(255,255,255,0,255,0, 255, 0, 255); break;
    case fx_palette_wcb: CreateTripleBand(255,255,255,0,255,255, 0, 0, 255); break;
    case fx_palette_wcm: CreateTripleBand(255,255,255,0,255,255, 255, 0, 255); break;
    case fx_palette_wbm: CreateTripleBand(255,255,255,255,0,255, 0, 0, 255); break;

    case fx_palette_rgb:CreateTripleBand(255,0,0, 0,255,0, 0,0,255);break;
  }
}

void setup() {
#if DEBUG  
  Serial.begin(9600); //serial communication at 9600 bauds
  Serial.println("On");
#endif  
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  currentPalette = CRGBPalette16(CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),
                                 CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),
                                 CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),
                                 CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0));

  initialPalette = CRGBPalette16(CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),
                                 CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),
                                 CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),
                                 CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0),CRGB(0,0,0));
  bootupTime = millis(); 
  timeDelay = bootupTime + timeDelay;
#if DEBUG                                 
  Serial.print("TimeDelay = ");
  Serial.println(timeDelay);
#endif
}

int GetNextTimeCodeMatch(int currentMatch)
{
  unsigned long currentMatchedTimecode = activeTracks[currentMatch].timecode;
  
  for (int i=0;i<numTracks;i++)
    if (activeTracks[i].timecode > currentMatchedTimecode)
      return i;
  return 0;
}

int GetCurrentTimeCodeMatch(unsigned long timecode)
{
  int match = 0;
  
  for (int i=0;i<numTracks;i++)
    if (activeTracks[i].timecode <= timecode)
      match = i;  
  return match;
}

#if DEBUG
void FxEventsSay(unsigned long timecode, unsigned long matchedTimecode,unsigned long nextMatchedTimecode)
{
    float tc = (float)matchedTimecode / (float)1000.0f;
    Serial.print(tc);
    Serial.print(" :");
    for (int i=0;i<numTracks;i++)
      if (activeTracks[i].timecode == matchedTimecode)
        FxEventSay(activeTracks[i].event);

    Serial.print(", next");
/*    
    Serial.print(" = ");
    for (int i=0;i<numTracks;i++)
      if (activeTracks[i].timecode == nextMatchedTimecode)
          FxEventSay(activeTracks[i].state);        
*/          
    Serial.print(" in ");
    float timeUntil = (float)(nextMatchedTimecode - (float)timecode) / 1000.0f;
    Serial.print(timeUntil);
    Serial.print("s");    
    Serial.println();
}
#endif

void FxEventPoll(unsigned long timecode)
{
  int match = GetCurrentTimeCodeMatch(timecode);
  int nextmatch = GetNextTimeCodeMatch(match);  
  unsigned long matchedTimecode = activeTracks[match].timecode;
  unsigned long nextMatchedTimecode = activeTracks[nextmatch].timecode;
  
  if (matchedTimecode > lastMatchedTimecode)
  {
    timedTransition = false;
#if DEBUG    
    FxEventsSay(timecode, matchedTimecode, nextMatchedTimecode);
#endif    

    for (int i=0;i<numTracks;i++)
      if (activeTracks[i].timecode == matchedTimecode)
        FxEventProcess(activeTracks[i].event);

    lastMatchedTimecode = timecode;
  }

  unsigned long totalSpan = nextMatchedTimecode - lastMatchedTimecode;  
  transition = ((float)timecode - (float)lastMatchedTimecode ) / (float)totalSpan;
}

uint8_t lerp(float mux, uint8_t a, uint8_t b) { return (uint8_t)(a * (1.0 - mux) + b * mux); }
CRGB LerpRGB(float t, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) { return CRGB(lerp(t, r1, r2),lerp(t, g1, g2),lerp(t, b1, b2)); }

void loop()
{
  FxEventPoll(millis()+timeDelay);
  
  if (timedTransition)
  {
    //Interpolate initial palette to next palette, based on transition (0 to 1)
    for (int i=0;i<16;i++)
    {
      CRGB rgb = LerpRGB(transition,
        initialPalette[i][0],initialPalette[i][1],initialPalette[i][2],
        nextPalette[i][0],nextPalette[i][1],nextPalette[i][2]);
      currentPalette[i] = rgb;
    } 
  }
  
  static uint8_t startIndex = 0;  
  startIndex = startIndex + (paletteDirection*paletteSpeed); /* motion speed */
  FillLEDsFromPaletteColors( startIndex);
  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void CreateSingleBand(uint8_t r, uint8_t g, uint8_t b) { return CreateQuadBand(r,g,b,r,g,b,r,g,b,r,g,b); }
void CreateDoubleBand(uint8_t r1, uint8_t g1, uint8_t b1,uint8_t r2, uint8_t g2, uint8_t b2) { return CreateQuadBand(r1,g1,b1,r2,g2,b2,r1,g1,b1,r2,g2,b2); }
void CreateTripleBand(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t r3, uint8_t g3, uint8_t b3) { return CreateQuadBand(r1,g1,b1,r2,g2,b2,r3,g3,b3,0,0,0); }
void CreateQuadBand(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t r3, uint8_t g3, uint8_t b3, uint8_t r4, uint8_t g4, uint8_t b4)
{
    uint8_t rh1 = r1/2; uint8_t gh1 = g1/2; uint8_t bh1 = b1/2;
    uint8_t rh2 = r2/2; uint8_t gh2 = g2/2; uint8_t bh2 = b2/2;
    uint8_t rh3 = r3/2; uint8_t gh3 = g3/2; uint8_t bh3 = b3/2;
    uint8_t rh4 = r4/2; uint8_t gh4 = g4/2; uint8_t bh4 = b4/2;
    if (fastTransition)
      currentPalette = CRGBPalette16(CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4));
    else 
    {
      nextPalette = CRGBPalette16(CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4));
      FxEventTransition();
    }
}
