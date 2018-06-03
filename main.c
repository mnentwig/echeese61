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
  //oscVol[30] = 0.01;
  audio_init();
  MIDI_init();

  //int count = 0;
  while (1){
    MIDI_run();
    audio_run();
    //printf("%i\n", (count++));
  }
  return 0;
}
