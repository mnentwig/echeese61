//  gcc -Wall -Wno-unused-variable main.c
#include <stdint.h>
typedef uint32_t u32;
#include <stdio.h>
#include <stdlib.h>
#include "include/generated.h"
float oscVol[NOSC];
#include "engine.c"
#include "audioDriver.c"
#include "midi.c"
#ifdef WINDOWS
#include "MIDI_windows.c"
#endif
#ifdef LINUX
#include "MIDI_linux.c"
#endif
int main(void){
  for (int ix = 0; ix < NOSC; ++ix){
    oscVol[ix] = 0;
  }
  portaudio_init();
  MIDI_init();
  
  while (1){
    Pa_Sleep( 1 * 1000 );
  }
  return 0;
}
