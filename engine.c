#include "include/oscillators.h"
uint32_t oscPhase[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#include "include/oscDeltaPhase.h"
int keyDown[256+128]; // should be volatile because interaction across interrupt
float oscVol[NOSC];

void engine_init(){
  for (int ix = 0; ix < NOSC; ++ix){
    oscVol[ix] = 0;
  }
  for (int ix = 0; ix < 128; ++ix)
    keyDown[ix] = 0;
}

void engine_on(int key){
  key &= 0x7F;
  keyDown[key+128] = 1;
}

void engine_off(int key){
  key &= 0x7F;
  keyDown[key+128] = 0;
}

void engine_updateOscVol(){  
  // === clear combined oscillator output ===
  int oscPts[NOSC];
  int* p1 = oscPts;
  int* p2;
  int* p3;
  int* p4;
  for (int ix = NOSC; ix != 0; --ix)
    *(p1++) = 0;
  
  int pts16 = 255;
  int pts8 = 255;
  int pts4 = 255;
  int ptsII = 255;
  int ptsIII = 255;
  
  // === accumulate contribution from 16'' drawbar ===
  p1 = oscPts;
  p2 = keyDown + 128 + 36;
  for (int ix = NOSC; ix > 0; --ix)
    *(p1++) += *(p2++) ? pts16 : 0.0f;  
  
  // === accumulate contribution from 8'' drawbar ===
  p1 = oscPts;
  p2 = keyDown + 128 + 36 - 12;
  for (int ix = NOSC; ix > 0; --ix)
    *(p1++) += *(p2++) ? pts8 : 0.0f;  

  // === accumulate contribution from 4'' drawbar ===
  p1 = oscPts;
  p2 = keyDown + 128 + 36 - 12 - 12;
  for (int ix = NOSC; ix > 0; --ix)
    *(p1++) += *(p2++) ? pts4 : 0.0f;  

  // === accumulate contribution from I drawbar ===
  p1 = oscPts;
  p2 = keyDown + 128 + 36 -12 -7;
  p3 = keyDown + 128 + 36 -12 -12 -12 -4;
  for (int ix = NOSC; ix > 0; --ix){
    *(p1) += *(p2++) ? ptsII : 0.0f;  
    *(p1) += *(p3++) ? ptsII : 0.0f;  
    ++p1;
  }

  // === accumulate contribution from II drawbar ===
  p1 = oscPts;
  p2 = keyDown + 128 + 36 -12 -12 -7;
  p3 = keyDown + 128 + 36 -12 -12 -12;
  p4 = keyDown + 128 + 36 -12 -12 -12 -12;
  for (int ix = NOSC; ix > 0; --ix){
    *(p1) += *(p2++) ? ptsIII : 0.0f;  
    *(p1) += *(p3++) ? ptsIII : 0.0f;  
    *(p1) += *(p4++) ? ptsIII : 0.0f;  
    ++p1;
  }
  
  // === map combined drawbar points to wave level (TBD) ===
  float* tmpF = oscVol;
  p1 = oscPts;
  for (int ix = NOSC; ix != 0; --ix){
    int pts = *(p1++);
    *(tmpF++) = pts > 0 ? 0.03 : 0.0001;
  }
}

void engine_run(float* outVal){
  engine_updateOscVol();
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
