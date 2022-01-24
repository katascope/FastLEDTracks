/* Time-Track system for FastLED Addressable LED lighting rigs, by Layne Thomas 1-21-2022-11:03
 *  This program can store an execute a millisecond-accurate lighting event system.
 *  
 *  See TracksLead and TracksFollow 
 *  To create a spectrum analysis for use in creating the events:
 *  Use ffmpeg with a size of (song length * 10). ex 205 seconds * 10 = 2050x512 as below.
 *   ex. ffmpeg -i "game.m4a" -lavfi showspectrumpic=s=2050x512:color=4:scale=lin:stop=8000 gameLd.png
*/

// Set 1 for Dance lead, 0 for Dance follow
#define LEAD 0

#include <FastLED.h>
#define LED_PIN     5
#define NUM_LEDS    205
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define SHOW_NEXT 0
#define UPDATES_PER_SECOND 100
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
CRGBPalette16 initialPalette;
TBlendType    currentBlending;
int timeDelay = 2800;//Bootup time less for later
int paletteSpeed = 1;
int paletteDirection = 1;
unsigned long lastMatchedTimecode = 0;
bool timedTransition = false;
CRGB transitionTo;
float transition = 0;


enum FxState
{
  fx_unknown = 0,
  fx_palette_black = 1,
  fx_palette_white = 2,
  fx_palette_red = 3,
  fx_palette_yellow = 4,
  fx_palette_green = 5,
  fx_palette_cyan = 6,
  fx_palette_blue = 7,
  fx_palette_magenta = 8,
  fx_palette_orange = 9,
  
  fx_palette_lead = 10,
  fx_palette_follow = 11,
  fx_palette_rgb = 12,
  fx_palette_whitered = 13,
  fx_palette_whiteyellow = 14,
  fx_palette_whitegreen = 15,
  fx_palette_whitecyan = 16,
  fx_palette_whiteblue = 17,
  fx_palette_whitemagenta = 18,
  fx_palette_redblue = 19,
  fx_palette_purplegreen = 20,

  fx_brightness_quarter = 40,
  fx_brightness_half = 41,
  fx_brightness_full = 42,

  fx_timed_black = 51,
  fx_timed_white = 52,
  fx_timed_red = 53,
  fx_timed_yellow = 54,
  fx_timed_green = 55,
  fx_timed_cyan = 56,
  fx_timed_blue = 57,
  fx_timed_magenta = 58,
  fx_timed_orange = 59,

  fx_speed_pos = 100,
  fx_speed_neg = 101,
  fx_speed_0 = 102,
  fx_speed_1 = 103,
  fx_speed_2 = 104,
  fx_speed_3 = 105,
  fx_speed_4 = 106,
  fx_speed_8 = 107,
  fx_speed_16 = 108
};
struct Fx { unsigned long timecode; FxState state;   };

// Main Track set to 'The Game Has Changed'

//Leads track
Fx TracksLead[] = 
{
  {1,fx_palette_black},
  {1,fx_speed_1},  
  {1,fx_speed_pos},
  {1,fx_timed_blue},
  
  {9633,fx_palette_red},
  {12033,fx_speed_2},
  {12033,fx_palette_blue},
  {14366,fx_palette_red},
  {16833,fx_palette_blue},

  //First build-up
  {19166,fx_palette_redblue},
  {19166,fx_speed_1},

  //Coast
  {24100,fx_palette_whiteblue},
  {26366,fx_palette_white},    
  {26900,fx_palette_whiteblue},

  //Build2
  {28733,fx_palette_rgb},
  {28733,fx_speed_1},

  //Coast2
  {33633,fx_palette_whiteblue},
  {35966,fx_palette_white},    
  {36466,fx_palette_whiteblue},

  //Build3
  {38400,fx_palette_rgb},
  {38400,fx_speed_2},

  {40766,fx_speed_pos},
  {40766,fx_speed_3},
  {43166,fx_speed_neg},
  {43166,fx_speed_4},  
  {45566,fx_speed_pos},
  {45566,fx_speed_8},

  //Breakthrough to the G
  {48100,fx_speed_8},
  {48100,fx_timed_magenta},
  {52766,fx_speed_0},
  {52766,fx_timed_cyan},
  {59933,fx_timed_yellow},
  {62366,fx_timed_orange},
  
  //coasting orange 
  {69566,fx_palette_orange},
  {69566,fx_speed_1},

//placeholder
  {80500, fx_palette_black},

  //the end
  {205000, fx_palette_black},
};

//Follows track
Fx TracksFollow[] = 
{
  {1,fx_palette_black},
  {1,fx_speed_1},  
  {1,fx_speed_pos},
  {1,fx_timed_red},
  
  {9633,fx_palette_blue},
  {12033,fx_speed_2},
  {12033,fx_palette_red},
  {14366,fx_palette_blue},
  {16833,fx_palette_red},

  //First build-up
  {19166,fx_palette_redblue},
  {19166,fx_speed_1},

  //Coast
  {24100,fx_palette_whitered},
  {26366,fx_palette_white},    
  {26900,fx_palette_whitered},

  //Build2
  {28733,fx_palette_rgb},
  {28733,fx_speed_1},

  //Coast2
  {33633,fx_palette_whitered},
  {35966,fx_palette_white},    
  {36466,fx_palette_whitered},

  //Build3
  {38400,fx_palette_rgb},
  {38400,fx_speed_2},

  {40766,fx_speed_neg},
  {40766,fx_speed_3},
  {43166,fx_speed_pos},
  {43166,fx_speed_4},  
  {45566,fx_speed_neg},
  {45566,fx_speed_8},

  //Breakthrough to the G
  {48100,fx_speed_8},
  {48100,fx_timed_magenta},
  {52766,fx_speed_0},
  {52766,fx_timed_cyan},
  {59933,fx_timed_yellow},
  {62366,fx_timed_orange},
  
  //coasting orange 
  {69566,fx_palette_orange},
  {69566,fx_speed_1},

//placeholder
  {80500, fx_palette_black},

  //the end
  {205000, fx_palette_black},
};


#if LEAD
const int numTracks = sizeof(TracksLead)/sizeof(Fx);
Fx *activeTracks = &TracksLead[0];
#else
const int numTracks = sizeof(TracksFollow)/sizeof(Fx);
Fx *activeTracks = &TracksFollow[0];
#endif

void FxStateSay(int state)
{  
  Serial.print(" ");
  switch(state)
  {
    case fx_unknown: Serial.print("unknown");break;
    case fx_palette_black: Serial.print("black");break;
    case fx_palette_white: Serial.print("white");break;
    case fx_palette_red: Serial.print("red");break;
    case fx_palette_yellow: Serial.print("yellow");break;
    case fx_palette_green: Serial.print("green");break;
    case fx_palette_cyan: Serial.print("cyan");break;
    case fx_palette_blue: Serial.print("blue");break;
    case fx_palette_magenta: Serial.print("magenta");break;
    case fx_palette_orange: Serial.print("orange");break;
    
    case fx_palette_lead: Serial.print("lead");break;    
    case fx_palette_follow: Serial.print("follow");break;    
    
    case fx_palette_rgb: Serial.print("rgb");break;
    case fx_palette_whitered: Serial.print("whitered");break;
    case fx_palette_whiteyellow: Serial.print("whiteyellow");break;
    case fx_palette_whitegreen: Serial.print("whitegreen");break;
    case fx_palette_whitecyan: Serial.print("whitecyan");break;
    case fx_palette_whiteblue: Serial.print("whiteblue");break;
    case fx_palette_whitemagenta: Serial.print("whitemagenta");break;
    case fx_palette_purplegreen: Serial.print("purplegreen");break;
    case fx_palette_redblue: Serial.print("redblue");break;

    case fx_brightness_quarter: Serial.print("brightness_25");break;
    case fx_brightness_half: Serial.print("brightness_50");break;
    case fx_brightness_full: Serial.print("brightness_75");break;

    case fx_timed_black: Serial.print("timed_black");break;
    case fx_timed_white: Serial.print("timed_white");break;
    case fx_timed_red: Serial.print("timed_red");break;
    case fx_timed_yellow: Serial.print("timed_yellow");break;
    case fx_timed_green: Serial.print("timed_green");break;
    case fx_timed_cyan: Serial.print("timed_cyan");break;
    case fx_timed_blue: Serial.print("timed_blue");break;
    case fx_timed_magenta: Serial.print("timed_magenta");break;
    case fx_timed_orange: Serial.print("timed_orange");break;

    case fx_speed_pos: Serial.print("pos");break;
    case fx_speed_neg: Serial.print("neg");break;
    case fx_speed_0: Serial.print("x0");break;
    case fx_speed_1: Serial.print("x1");break;
    case fx_speed_2: Serial.print("x2");break;
    case fx_speed_3: Serial.print("x3");break;
    case fx_speed_4: Serial.print("x4");break;
    case fx_speed_8: Serial.print("x8");break;
    case fx_speed_16: Serial.print("x16");break;
  }
}
void FxStateProcess(int state)
{
  switch (state)
  {
    case fx_unknown: Serial.println("Unknown Found!");break;
   
    case fx_palette_black:CreateSingleBand(0,0,0);break;
    case fx_palette_white:CreateSingleBand(255,255,255);break;
    case fx_palette_red:CreateSingleBand(255,0,0);break;
    case fx_palette_yellow:CreateSingleBand(255,255,0);break;
    case fx_palette_green:CreateSingleBand(0,255,0);break;
    case fx_palette_cyan:CreateSingleBand(0,255,255);break;
    case fx_palette_blue:CreateSingleBand(0,0,255);break;
    case fx_palette_magenta:CreateSingleBand(255,0,255);break;
    case fx_palette_orange:CreateSingleBand(255,127,0);break;
    
    case fx_palette_lead:CreateQuadBand(255,0,0,255,0,0,255,0,0, 0,0,255);break;
    case fx_palette_follow:CreateQuadBand(0,0,255, 0,0,255, 0,0,255, 255,0,0);break;
    
    case fx_palette_rgb:CreateTripleBand(255,0,0, 0,255,0, 0,0,255);break;
    case fx_palette_whitered:CreateDoubleBand(255,0,0,192,192,192);break;
    case fx_palette_whiteyellow:CreateDoubleBand(255,255,0,192,192,192);break;
    case fx_palette_whitegreen:CreateDoubleBand(0,255,0,192,192,192);break;
    case fx_palette_whitecyan:CreateDoubleBand(0,255,255,192,192,192);break;
    case fx_palette_whiteblue:CreateDoubleBand(0,0,255,192,192,192);break;
    case fx_palette_whitemagenta:CreateDoubleBand(255,0,255,192,192,192);break;
    
    case fx_palette_purplegreen:CreateDoubleBand(255,0,255, 0,255,0);break;
    case fx_palette_redblue:CreateDoubleBand(255,0,0,0,0,255);break;

    case fx_timed_black:timedTransition = true;GrabPalette();transitionTo = CRGB(0,0,0);break;
    case fx_timed_white:timedTransition = true;GrabPalette();transitionTo = CRGB(255,255,255);break;
    case fx_timed_red:timedTransition = true;GrabPalette();transitionTo = CRGB(255,0,0);break;
    case fx_timed_yellow:timedTransition = true;GrabPalette();transitionTo = CRGB(255,255,0);break;
    case fx_timed_green:timedTransition = true;GrabPalette();transitionTo = CRGB(0,255,0);break;
    case fx_timed_cyan:timedTransition = true;GrabPalette();transitionTo = CRGB(0,255,255);break;
    case fx_timed_blue:timedTransition = true;GrabPalette();transitionTo = CRGB(0,0,255);break;
    case fx_timed_magenta:timedTransition = true;GrabPalette();transitionTo = CRGB(255,0,255);break;
    case fx_timed_orange:timedTransition = true;GrabPalette();transitionTo = CRGB(255,127,0);break;

    case fx_brightness_quarter: FastLED.setBrightness(  BRIGHTNESS );
    case fx_brightness_half: FastLED.setBrightness(  127 );
    case fx_brightness_full: FastLED.setBrightness(  192 );

    case fx_speed_pos:paletteDirection = 1;break;
    case fx_speed_neg:paletteDirection = -1;break;
    case fx_speed_0:paletteSpeed = 0;break;
    case fx_speed_1:paletteSpeed = 1;break;
    case fx_speed_2:paletteSpeed = 2;break;
    case fx_speed_3:paletteSpeed = 3;break;
    case fx_speed_4:paletteSpeed = 4;break;
    case fx_speed_16:paletteSpeed = 8;break;
  }
}

void setup() {
    Serial.begin(9600); //serial communication at 9600 bauds
    Serial.println("Activating LED Game Grid");
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
                                   
    currentBlending = LINEARBLEND;
    Serial.println("LEDs synchronized and activated. Proceed to dance.");
}

void GrabPalette()
{
  initialPalette = CRGBPalette16( 
    currentPalette[0], currentPalette[1], currentPalette[2], currentPalette[3], 
    currentPalette[4], currentPalette[5], currentPalette[6], currentPalette[7], 
    currentPalette[8], currentPalette[9], currentPalette[10], currentPalette[11], 
    currentPalette[12], currentPalette[13], currentPalette[14], currentPalette[15]);
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

void FxStatePoll(unsigned long timecode)
{
  int match = GetCurrentTimeCodeMatch(timecode);
  int nextmatch = GetNextTimeCodeMatch(match);
  
  unsigned long matchedTimecode = activeTracks[match].timecode;
  unsigned long nextMatchedTimecode = activeTracks[nextmatch].timecode;
  
  if (matchedTimecode > lastMatchedTimecode)
  {
    timedTransition = false;
    float tc = (float)matchedTimecode / (float)1000.0f;
    Serial.print(tc);
    Serial.print(" :");
    for (int i=0;i<numTracks;i++)
    {
      if (activeTracks[i].timecode == matchedTimecode)
      {
        FxStateSay(activeTracks[i].state);
        FxStateProcess(activeTracks[i].state);
      }
    }
#if SHOW_NEXT    
    Serial.print(", next=(");
    for (int i=0;i<numTracks;i++)
      if (Tracks[i].timecode == nextMatchedTimecode)
          FxStateSay(Tracks[i].state);        
    Serial.print(" in ");

    float timeUntil = (float)(nextMatchedTimecode - (float)timecode) / 1000.0f;
    Serial.print(timeUntil);
    Serial.print("s)");    
#endif    
    Serial.println();
    lastMatchedTimecode = timecode;
  }

  unsigned long totalSpan = nextMatchedTimecode - lastMatchedTimecode;  
  transition = ((float)timecode - (float)lastMatchedTimecode ) / (float)totalSpan;
}

uint8_t lerp(float mux, uint8_t a, uint8_t b) { return (uint8_t)(a * (1.0 - mux) + b * mux); }
CRGB LerpRGB(float t, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) { return CRGB(lerp(t, r1, r2),lerp(t, g1, g2),lerp(t, b1, b2)); }

void loop()
{
  FxStatePoll(millis()+timeDelay);
  if (timedTransition)
  {
    for (int i=0;i<16;i++)
    {
      CRGB rgb = LerpRGB(transition,
        initialPalette[i][0],initialPalette[i][1],initialPalette[i][2],
        transitionTo[0],transitionTo[1],transitionTo[2]);
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

void CreateSingleBand(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t rh = r/2; uint8_t gh = g/2; uint8_t bh = b/2;
    currentPalette = CRGBPalette16(CRGB(r,g,b),CRGB(rh,gh,bh),CRGB(r,g,b),CRGB(rh,gh,bh),
                                   CRGB(r,g,b),CRGB(rh,gh,bh),CRGB(r,g,b),CRGB(rh,gh,bh),
                                   CRGB(r,g,b),CRGB(rh,gh,bh),CRGB(r,g,b),CRGB(rh,gh,bh),
                                   CRGB(r,g,b),CRGB(rh,gh,bh),CRGB(r,g,b),CRGB(rh,gh,bh));
}

void CreateDoubleBand(uint8_t r1, uint8_t g1, uint8_t b1,uint8_t r2, uint8_t g2, uint8_t b2)
{
    uint8_t rh1 = r1/2; uint8_t gh1 = g1/2; uint8_t bh1 = b1/2;
    uint8_t rh2 = r2/2; uint8_t gh2 = g2/2; uint8_t bh2 = b2/2;
    currentPalette = CRGBPalette16(CRGB(r1,g1,b1),CRGB(rh1,gh1,bh1),CRGB(r2,g2,b2),CRGB(rh2,gh2,bh2),
                                  CRGB(r1,g1,b1),CRGB(rh1,gh1,bh1),CRGB(r2,g2,b2),CRGB(rh2,gh2,bh2),
                                  CRGB(r1,g1,b1),CRGB(rh1,gh1,bh1),CRGB(r2,g2,b2),CRGB(rh2,gh2,bh2),
                                  CRGB(r1,g1,b1),CRGB(rh1,gh1,bh1),CRGB(r2,g2,b2),CRGB(rh2,gh2,bh2));                                  
}

void CreateTripleBand(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t r3, uint8_t g3, uint8_t b3)
{
    uint8_t rh1 = r1/2; uint8_t gh1 = g1/2; uint8_t bh1 = b1/2;
    uint8_t rh2 = r2/2; uint8_t gh2 = g2/2; uint8_t bh2 = b2/2;
    uint8_t rh3 = r3/2; uint8_t gh3 = g3/2; uint8_t bh3 = b2/2;
    currentPalette = CRGBPalette16(CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(0,0,0),
                                  CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(0,0,0),
                                  CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(0,0,0),
                                  CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(0,0,0));
}
void CreateQuadBand(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t r3, uint8_t g3, uint8_t b3, uint8_t r4, uint8_t g4, uint8_t b4)
{
    uint8_t rh1 = r1/2; uint8_t gh1 = g1/2; uint8_t bh1 = b1/2;
    uint8_t rh2 = r2/2; uint8_t gh2 = g2/2; uint8_t bh2 = b2/2;
    uint8_t rh3 = r3/2; uint8_t gh3 = g3/2; uint8_t bh3 = b3/2;
    uint8_t rh4 = r4/2; uint8_t gh4 = g4/2; uint8_t bh4 = b4/2;
    currentPalette = CRGBPalette16(CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                  CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                  CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                  CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4));
}
