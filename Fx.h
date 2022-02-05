#if !defined FX_DEF
#define FX_DEF

/* 
 * 
 */
#include <FastLED.h>

static uint8_t lerp(float mux, uint8_t a, uint8_t b) { return (uint8_t)(a * (1.0 - mux) + b * mux); }
static CRGB LerpRGB(float t, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) { return CRGB(lerp(t, r1, r2),lerp(t, g1, g2),lerp(t, b1, b2)); }

#define DARK    0x00,0x00,0x00
#define WHITE   0xFF,0xFF,0xFF
#define RED     0xFF,0x00,0x00
#define YELLOW  0xFF,0xFF,0x00
#define GREEN   0x00,0xFF,0x00
#define CYAN    0x00,0xFF,0xFF
#define BLUE    0x00,0x00,0xFF
#define MAGENTA 0xFF,0x00,0xFF
#define ORANGE  0xFF,0x7F,0x00

enum FxState
{
  FxState_Default       = 0,
  FxState_TestPattern   = 1,
  FxState_PlayingTrack  = 2,
};

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
  fx_speed_32 = 19,

  fx_speed_pos = 20,
  fx_speed_neg = 21,

  fx_transition_fast = 30,
  fx_transition_timed = 31,  

  fx_palette_lead = 40,
  fx_palette_follow = 41,

  fx_track_begin = 50,
  fx_track_stop = 51,

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

static String FxEventName(int event)
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
    case fx_speed_32: return F("x32");break;
    
    case fx_transition_timed:return F("timed");break;
    case fx_transition_fast: return F("fast");break;
    case fx_palette_lead:    return F("lead");break;    
    case fx_palette_follow:  return F("follow");break;       

    case fx_palette_dark:    return F("dark");break;
    case fx_palette_white:   return F("white");break;
    case fx_palette_red:     return F("red");break;
    case fx_palette_yellow:  return F("yellow");break;
    case fx_palette_green:   return F("green");break;
    case fx_palette_cyan:    return F("cyan");break;
    case fx_palette_blue:    return F("blue");break;
    case fx_palette_magenta: return F("magenta");break;
    case fx_palette_orange:  return F("orange");break;
   
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
    
    case fx_palette_rgb:return F("rgb");break;
  }
}

struct FxController
{
  CRGBPalette16 currentPalette;
  CRGBPalette16 initialPalette;
  CRGBPalette16 nextPalette;
  bool timedTransition = false;
  int paletteSpeed = 1;
  int paletteDirection = 1;
};
static FxController fxController;

void CreateTimedTransition()
{ 
  fxController.initialPalette = fxController.currentPalette; 
  fxController.timedTransition = true; 
}

void CreateQuadBand(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t r3, uint8_t g3, uint8_t b3, uint8_t r4, uint8_t g4, uint8_t b4)
{
    if (fxController.timedTransition)
    {
      fxController.nextPalette = CRGBPalette16(CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                   CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4));
      CreateTimedTransition();
    }
    else  
    {
      fxController.currentPalette = CRGBPalette16(CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                    CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                    CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4),
                                    CRGB(r1,g1,b1),CRGB(r2,g2,b2), CRGB(r3,g3,b3),CRGB(r4,g4,b4));
    }
}
void CreateTripleBand(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t r3, uint8_t g3, uint8_t b3) { return CreateQuadBand(r1,g1,b1,r2,g2,b2,r3,g3,b3,0,0,0); }
void CreateDoubleBand(uint8_t r1, uint8_t g1, uint8_t b1,uint8_t r2, uint8_t g2, uint8_t b2) { return CreateQuadBand(r1,g1,b1,r2,g2,b2,r1,g1,b1,r2,g2,b2); }
void CreateSingleBand(uint8_t r, uint8_t g, uint8_t b) { return CreateQuadBand(r,g,b,r,g,b,r,g,b,r,g,b); }


void FxEventProcess(int event)
{  
  switch (event)
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
      fxController.paletteSpeed = event;
      break;
    case fx_speed_32:
      fxController.paletteSpeed = 32;
      break;

    case fx_speed_pos:fxController.paletteDirection = 1;break;
    case fx_speed_neg:fxController.paletteDirection = -1;break;

    case fx_transition_fast:fxController.timedTransition = false;break;
    case fx_transition_timed:fxController.timedTransition = true;break;

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

#endif
