#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>

// ***************************************************************
// MIDI handler (Windows OS)
// ***************************************************************
// application must implement the following callback functions for MIDI_parse:
void MIDI_noteon(u32 chan, u32 key, u32 vel);
void MIDI_noteoff(u32 chan, u32 key);
void MIDI_cmd(u32 chan, u32 cc, u32 val);


typedef unsigned char u8;
void CALLBACK midiCallback(HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2){
  if (uMsg == MIM_DATA){
    u8 b0 = (dwParam1 >>  0) & 0xFF;
    u8 b1 = (dwParam1 >>  8) & 0xFF;
    u8 b2 = (dwParam1 >> 16) & 0xFF;
    
    if ((b0 & 0xF0) == 0x80){
      //printf("noteoff %i %i\n", b1, b2);
      MIDI_noteoff(b0 & 0x0F, b1);
    } else if ((b0 & 0xF0) == 0x90){
      //printf("noteon %i %i\n", b1, b2);
      MIDI_noteon(b0 & 0x0F, b1, b2);
    } else if ((b0 & 0xF0) == 0xb0){
      MIDI_cmd(b0 & 0x0F, b1, b2);
    }
  }
}

void MIDI_init(void){
  unsigned long result;
  HMIDIIN      inHandle;
  result = midiInOpen(&inHandle, 0, (DWORD)midiCallback, 0, CALLBACK_FUNCTION);
  if (result){
    printf("MIDI in open failed\n");
    exit(EXIT_FAILURE);
  }
  midiInStart(inHandle);  
}

