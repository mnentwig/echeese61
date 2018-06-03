void MIDI_noteon(u32 chan, u32 key, u32 vel){
  int k = key - 48;
  if ((k >= 0) && (k < NOSC)){
    oscVol[k] = 0.1;
  }
}
void MIDI_noteoff(u32 chan, u32 key){
  int k = key - 48;
  if ((k >= 0) && (k < NOSC)){
    oscVol[k] = 0;
  }
}

void MIDI_cmd(u32 chan, u32 cc, u32 val){

}
