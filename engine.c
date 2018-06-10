#include "include/oscillators.h"
uint32_t oscPhase[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#include "include/oscDeltaPhase.h"
volatile float oscVol[NOSC];

void engine_init(){
  for (int ix = 0; ix < NOSC; ++ix){
    oscVol[ix] = 0;
  }
}

void engine_on(int key){
  if ((key < 0) || (key >= NOSC))return;
  // note: This is fullscale => single key is clean, multiple keys clip
  oscVol[key] = 1;
}

void engine_off(int key){
  if ((key < 0) || (key >= NOSC))return;
  oscVol[key] = 0;
}

void engine_run(float* outVal){
  float oscOut[NOSC];

  uint32_t* p1 = oscPhase;
  uint32_t* p2 = oscDeltaPhase;
  for (int ix = 12; ix != 0; --ix)
    *(p1++) += *(p2++);
  
  float* pSpline = oscSpline; // iterate over all splines
  float* pOut = oscOut; // iterate over all outputs
  float sum = 0;
  int count = 0;
  for (int shift = 0; shift < 4; ++shift){
    uint32_t* p3 = oscPhase; // iterate over all oscillators
    for (int ix = 12; ix != 0; --ix){
      // === look up oscillator phase (max. division factor) ===
      uint32_t ph = *(p3++);
      
      // === undo division ===
      ph <<= shift;
      
      // === map to spline ===
      uint32_t intPart = ph >> 25;
      float fracPart = (float)(ph & 0x01FFFFFF)/(float)0x01000000-1.0f;
      
      // === look up spline segment ===
      float* pL = pSpline + 4*intPart;
      
      // === next spline ===
      pSpline += (SPLINE_NPERIODS * SPLINE_SEGPERPERIOD * 4);
      
      // === Horner scheme ===
      float acc = *(pL++); // c3
      acc *= fracPart;
      acc += *(pL++); // c2
      acc *= fracPart;
      acc += *(pL++); // c1
      acc *= fracPart;
      acc += *pL; // c0
      *(pOut++) = acc;
      sum += oscVol[count++] * acc;
    } // for oscillator
  } // for divider
  *outVal = sum;
}
