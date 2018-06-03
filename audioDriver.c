#include <stdio.h>
#include <math.h>
#include "portaudio.h"

#define FRAMES_PER_BUFFER  (8)
//#define FRAMES_PER_BUFFER  (1024)
//#define FRAMES_PER_BUFFER  (256)
static int pa_callback(const void *inputBuffer, void *outputBuffer,
			  unsigned long framesPerBuffer,
			  const PaStreamCallbackTimeInfo* timeInfo,
			  PaStreamCallbackFlags statusFlags,
			  void *userData ){
  float *out = (float*)outputBuffer;
  
  (void) timeInfo; /* Prevent unused variable warnings. */
  (void) statusFlags;
  (void) inputBuffer;
  
  for(int ix=0; ix<framesPerBuffer; ++ix){
    float val;
    engine_run(&val);
    *out++ = val;
    *out++ = val;
  }    
  return paContinue;
}

PaStream *stream;
void portaudio_init(void){
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
