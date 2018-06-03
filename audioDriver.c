#include <stdio.h>
#include <math.h>
#if false
#include "portaudio.h"

#define FRAMES_PER_BUFFER  (16)
//#define FRAMES_PER_BUFFER  (1024)
//#define FRAMES_PER_BUFFER  (256)
double idle = 0;
PaStreamCallbackFlags f = -1;
int count =0;
static int pa_callback(const void *inputBuffer, void *outputBuffer,
		       unsigned long framesPerBuffer,
		       const PaStreamCallbackTimeInfo* timeInfo,
		       PaStreamCallbackFlags statusFlags,
		       void *userData ){
  float *out = (float*)outputBuffer;
  
  (void) timeInfo; /* Prevent unused variable warnings. */
  (void) statusFlags;
  (void) inputBuffer;
  if (statusFlags != f){
    printf("%li\n", statusFlags);
    f = statusFlags;
  }
  ++count;
  if (count % 1000 == 0)
    printf("%i\n", count);
  
  
  for(int ix=0; ix<framesPerBuffer; ++ix){
    float val;
    engine_run(&val);
    //val =0;
    val += idle;
    idle += 0.0001;
    if (idle > 0.001)
      idle -= 0.002;
    *out++ = val;
    *out++ = val;
  }    
  return paContinue;
}

PaStream *stream;
void audio_init(void){
  PaStreamParameters outputParameters;
  PaError err;
  
  err = Pa_Initialize(); if( err != paNoError ) goto error;
  
  outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
  // printf("portaudio device = %i\n", outputParameters.device);
  
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default output device.\n");
    goto error;
  }
  outputParameters.channelCount = 2;
  outputParameters.sampleFormat = paFloat32;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;
  
  err = Pa_OpenStream
    (&stream,
     NULL, /* no input */
     &outputParameters,
     96000/2,
     FRAMES_PER_BUFFER,
     paClipOff,
     pa_callback,
     NULL); if( err != paNoError ) goto error;
  err = Pa_StartStream(stream); if( err != paNoError ) goto error;
  
  // err = Pa_StopStream( stream ); if( err != paNoError ) goto error;
  // err = Pa_CloseStream( stream ); if( err != paNoError ) goto error;
  //Pa_Terminate();
  return;
 error:
  Pa_Terminate();
  fprintf( stderr, "Portaudio error:\n" );
  fprintf( stderr, "Error number: %d\n", err );
  fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
  exit(EXIT_FAILURE);
}
#else
#include <alloca.h>
#include <alsa/asoundlib.h>

snd_pcm_t *pcm_handle;
float *buff;
snd_pcm_uframes_t frames;
void audio_init() {
  unsigned int tmp;
  snd_pcm_hw_params_t *params;
  int buff_size;
  
  unsigned int rate 	 = 48000;
  int channels = 2;
	
  int ret;
  ret = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0); assert(ret >= 0);
  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(pcm_handle, params);
  ret = snd_pcm_hw_params_set_access(pcm_handle, params,SND_PCM_ACCESS_RW_INTERLEAVED); assert(ret >= 0);
  ret = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_FLOAT_LE); assert(ret >= 0);
  ret = snd_pcm_hw_params_set_channels(pcm_handle, params, 2);  assert(ret >= 0);
  ret = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0); assert(ret >= 0);
  int periods = 2;
  if (snd_pcm_hw_params_set_periods(pcm_handle, params, periods, 0) < 0) {
    fprintf(stderr, "Error setting periods.\n");
    exit(EXIT_FAILURE);
  }
#if false
  unsigned int latency_us = 50000;
  ret = snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_FLOAT_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, 48000, /*soft resample*/0, latency_us);
  assert(ret >= 0);
#endif
  
  ret = snd_pcm_hw_params(pcm_handle, params); assert(ret >= 0);
  	
  printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));
  printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));


  snd_pcm_hw_params_get_rate(params, &tmp, 0);
  printf("rate: %d bps\n", tmp);

  /* Allocate buffer to hold single period */
  ret = snd_pcm_hw_params_get_period_size(params, &frames, 0); assert(ret >= 0);

  buff_size = frames * channels * sizeof(float);
  buff = (float *) malloc(buff_size);
  printf("frames: %li\n", frames);
  
  snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
}

float v = 0;
void audio_run(){
  frames = 16;
  for(int ix=0; ix<frames; ++ix){
    float val;
    engine_run(&val);
    //val = v;
    v = v + 0.01;
    if (v > 1)
      v -= 2;
    buff[2*ix] = val;
    buff[2*ix+1] = val;
  }
  int pcm;
  if ((pcm = snd_pcm_writei(pcm_handle, buff, frames)) == -EPIPE) {
    printf("XRUN.\n");
    snd_pcm_prepare(pcm_handle);
  } else if (pcm < 0) {
    printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
  }
}
#endif
