#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <alsa/asoundlib.h>     /* Interface to the ALSA system */

// ***************************************************************
// MIDI parser
// ***************************************************************
// application must implement the following callback functions for MIDI_parse:
void MIDI_noteon(u32 chan, u32 key, u32 vel);
void MIDI_noteoff(u32 chan, u32 key);
void MIDI_cmd(u32 chan, u32 cc, u32 val);

// MIDI state machine
u32 MIDI_state;
u32 MIDI_mCmd;
u32 MIDI_mArg1;
u32 MIDI_mChan;
u32 MIDI_noteState[128];

enum {eMIDI_cmd, eMIDI_arg1, eMIDI_arg2, eMIDI_sysex};
snd_rawmidi_t* midiin = NULL;
void MIDI_init(void){
  MIDI_state = eMIDI_cmd;
  u32 ix;
  for (ix=0; ix < 128; ++ix)
    MIDI_noteState[ix] = 0;

  int status;
  int mode = SND_RAWMIDI_SYNC;// |    SND_RAWMIDI_NONBLOCK;
  const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
  if ((status = snd_rawmidi_open(&midiin, NULL, portname, mode)) < 0) {
    fprintf(stderr, "Problem opening MIDI input: %s", snd_strerror(status));
    exit(EXIT_FAILURE);
  }
  status = snd_rawmidi_nonblock(midiin, 1);
  if (status < 0){
    fprintf(stderr, "Problem setting MIDI nonblock mode: %s", snd_strerror(status));
    exit(EXIT_FAILURE);    
  }
}

void MIDI_parse(u32 b){
  if (b == 0xF0){
    // === enter sysex ===
    MIDI_state = eMIDI_sysex;
    return;
  } else if (b == 0xF7){
    // === leave sysex ===
    MIDI_state = eMIDI_cmd;
    return;
  }
  
  // === ignore other other system bytes ===
  u32 nh = b & 0xF0;
  if (nh == 0xF0)
    return;

  // === command byte ===
  if (nh & 0x80){
    MIDI_state = eMIDI_arg1;
    MIDI_mCmd = nh;
    MIDI_mChan = b & 0xF;
    return;
  }

  u32 tmp;
  switch (MIDI_state){
  case eMIDI_cmd:
    // === headerless data ===
    printf("lone MIDI data %02x\n", (unsigned int)b);
    return;

  case eMIDI_arg1:
    // === argument 1 ===
    MIDI_mArg1 = b & 0x7F;
    MIDI_state = eMIDI_arg2;
    if (MIDI_mCmd == 0xd0){
      // aftertouch
      MIDI_state = eMIDI_arg1;
    }
    return;
    
  case eMIDI_arg2:
    // === argument 2 ===
    MIDI_state = eMIDI_arg1; // running status: next is arg1
    
    // === convert zero-velocity noteon to noteoff ===
    // (do not change MIDI_mCmd, as running status remains unchanged)
    tmp = (MIDI_mCmd == 0x90) && (b == 0) ? 0x80 : MIDI_mCmd;    
    switch (tmp){
    case 0x80:
      // === noteoff ===
      if (MIDI_noteState[MIDI_mArg1]){
	MIDI_noteState[MIDI_mArg1] = 0;
	MIDI_noteoff(MIDI_mChan, MIDI_mArg1);
      }
      break;
      
    case 0x90:
      // === noteon ===
      if (!MIDI_noteState[MIDI_mArg1]){
	MIDI_noteState[MIDI_mArg1] = 1;
	MIDI_noteon(MIDI_mChan, MIDI_mArg1, b & 0x7F);
      }
      break;
      
    default:
      MIDI_cmd(MIDI_mChan, MIDI_mArg1, b);
      //printf("cmd %02x %02x %02x\n", MIDI_mCmd, MIDI_mArg1, b);
      break;
    }
  case eMIDI_sysex:
    break;
  }
}

void MIDI_run(){
  char buffer[256];
  while (1){
    int status = snd_rawmidi_read(midiin, buffer, sizeof(buffer));
    if (status < 0) {
      // fprintf(stderr, "Problem reading MIDI input: %s", snd_strerror(status));
      return;
    }
    if (status == 0)
      return;
    for (int ix = 0; ix < status; ++ix)
      MIDI_parse(buffer[ix]);
    if (status < sizeof(buffer))
      return;
  }
}
