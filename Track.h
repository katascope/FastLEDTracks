#if !defined TRACK_DEF
#define TRACK_DEF
#include <avr/pgmspace.h> 

#define LEAD      1                // Set 1 for Dance lead, 0 for Dance follow

// Main Track set to 'The Game Has Changed'
#define TRACK_START_DELAY    1800  // Delay time from start until track should truly 'start'
#if LEAD 
const unsigned long SongTrack[] PROGMEM = 
{
//Basic setup, fade in to blue
  1,fx_palette_dark,
  1,fx_speed_0,  
  1,fx_speed_pos,
  
//First snapin
  9633,fx_transition_timed_wipe_pos,
  9633,fx_palette_blue,
  //9633,fx_transition_timed_fade,
  //9633,fx_palette_dark,
//Second snapint  
  12033,fx_transition_timed_wipe_neg,
  12033,fx_palette_red,
  //12033,fx_transition_timed_fade,
  //12033,fx_palette_dark,
//Third snapin  
  14366,fx_transition_timed_wipe_pos,
  14366,fx_palette_blue,
  //14366,fx_transition_timed_fade,
  //14366,fx_palette_dark,
//Fourth snapin
  16833,fx_transition_timed_wipe_neg,
  16833,fx_palette_red,
  //16833,fx_transition_timed_fade,
  //16833,fx_palette_dark,

//March together
  19166,fx_transition_timed_fade,
  19166,fx_palette_rb,
  19166,fx_transition_timed_fade,
  19166,fx_palette_drb,
//  19166,fx_speed_1,  

  //Coast
  24100,fx_transition_timed_fade,
  24100,fx_palette_blue,
  
  26366,fx_transition_timed_wipe_pos,
  26366,fx_palette_white,   
  26900,fx_transition_timed_fade, 
  26900,fx_palette_db,

  //Build2
  28733,fx_transition_timed_fade, 
  28733,fx_palette_rb,

  //Coast2
  33633,fx_transition_timed_fade,
  33633,fx_palette_cyan,
  35966,fx_transition_timed_wipe_pos,
  35966,fx_palette_white,    
  36466,fx_transition_timed_fade,    
  36466,fx_palette_db,

  //Build3
  38400,fx_transition_timed_fade,
  38400,fx_palette_rb,
  40766,fx_transition_timed_fade,
  40766,fx_palette_db,
  43166,fx_transition_timed_fade,
  43166,fx_palette_rb,
  45566,fx_transition_timed_fade,
  45566,fx_palette_dr,

  //Breakthrough to the G
  48100,fx_speed_0,  
  48100,fx_transition_timed_fade,
  48100,fx_palette_magenta,
  52766,fx_transition_timed_fade,
  52766,fx_palette_cyan,
  59933,fx_transition_timed_fade,
  59933,fx_palette_yellow,
  62366,fx_transition_timed_fade,
  62366,fx_palette_orange,
  
  //coasting orange 
  69566,fx_transition_timed_fade,
  69566,fx_palette_rgb,
  69566,fx_speed_1,
  //69566,fx_transition_fast,

//placeholder
  80500,fx_transition_timed_fade,
  80500, fx_palette_dark,

  //the end
  205000, fx_palette_dark
};
#else //Follow
const unsigned long SongTrack[] PROGMEM = 
{
//Setup 
  1,fx_palette_dark,

  //1000,fx_transition_timed_wipe_pos,
  500,fx_palette_dark,

  1000,fx_transition_timed_wipe_neg,
  1000,fx_palette_blue,

  4000,fx_transition_timed_wipe_neg,
  4000,fx_palette_magenta,
  
  7000,fx_transition_timed_wipe_neg,
  7000,fx_palette_red,

  10000,fx_transition_timed_fade,
  10000,fx_palette_green,
  /*
  13000,fx_transition_timed_fade,
  13000,fx_palette_dark,

  15000,fx_transition_timed_wipe_pos,
  15000,fx_palette_rgb,*/
  13000,fx_transition_timed_fade,
  13000,fx_palette_white,

//placeholder
  80500,fx_transition_timed_fade,
  80500, fx_palette_dark,

  //the end
  205000, fx_palette_dark
};
#endif
const PROGMEM int numSongTracks = sizeof(SongTrack)/(sizeof(unsigned long)*2);

static unsigned long SongTrack_timecode(int i) { return pgm_read_dword(&(SongTrack[i*2+0])); } 
static unsigned long SongTrack_event(int i) {  return pgm_read_dword(&(SongTrack[i*2+1])); }

static int GetNextTimeCodeMatch(int currentMatch) { unsigned long tc = SongTrack_timecode(currentMatch); for (int i=0;i<numSongTracks;i++) if (SongTrack_timecode(i) > tc) return i; return 0; }
static int GetCurrentTimeCodeMatch(unsigned long timecode) { int match = 0; for (int i=0;i<numSongTracks;i++) { if (SongTrack_timecode(i) <= timecode) match = i; } return match; }

void FxTrackSay(unsigned long timecode, unsigned long matchedTimecode,unsigned long nextMatchedTimecode)
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
    Serial.print(F(", next = "));  
    for (int i=0;i<numSongTracks;i++)
    {
      if (SongTrack_timecode(i) == nextMatchedTimecode)
      {
          Serial.print(F(" "));
          Serial.print(FxEventName(SongTrack_event(i)));
      }
    }
  
    float timeUntil = (float)(nextMatchedTimecode - (float)timecode) / 1000.0f;
    Serial.print(F(" in "));
    Serial.print(timeUntil);
    Serial.print(F("s"));
    Serial.println();
}

#endif
