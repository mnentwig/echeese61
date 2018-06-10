#include <circle/startup.h>
#include <circle/devicenameservice.h>
#include <circle/usb/usbmidi.h>
#include <circle/pwmsoundbasedevice.h>
#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
//#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/types.h>
#include <circle/usb/dwhcidevice.h>

// === program logic ===
typedef u32 uint32_t;
#include "include/generated.h"
#include "engine.c"

// === app wrapper class ===
// main purpose: provide callback function for sound device (must separate from CKernel)
class CApp : public CPWMSoundBaseDevice {
public:
  CApp (CInterruptSystem* m_Interrupt);
  ~CApp(void);
  boolean Initialize (void);
private:
  unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
  float sampleScale;
  float sampleOffset;
};

// Note: 128 samples latency at 48ksps seems to be the fastest mode supported for PWM audio (64 samples or 96 ksps gives bad drop
CApp::CApp(CInterruptSystem* m_Interrupt) : CPWMSoundBaseDevice(m_Interrupt, /*sample rate*/48000, /*latency (chunk size)*/256){
  // get sample scaling from sound device
  this->sampleScale = (float)(this->GetRangeMax())/2;
  this->sampleOffset = (float)(this->GetRangeMax())/2;
};
CApp::~CApp(void){}

// === audio callback ===
unsigned CApp::GetChunk (u32 *pBuffer, unsigned nChunkSize){
  for (int ix = nChunkSize; ix > 0; ix -= 2){
    float val;
    engine_run(&val);
    u32 tmp = val * this->sampleScale + this->sampleOffset;//((val+1)*(float)this->GetRangeMax()/2);
    *(pBuffer++) = tmp;
    *(pBuffer++) = tmp;
  }
  return nChunkSize;
}

// === "Kernel" class (access to circle functionality) ===
class CKernel{
public:
  CKernel (void);
  ~CKernel (void);
  boolean Initialize(void);
  int Run(void);  
private:
  // do not change this order
  CMemorySystem		m_Memory;
  CActLED			m_ActLED;
  CKernelOptions		m_Options;
  CDeviceNameService	m_DeviceNameService;
  CScreenDevice		m_Screen;
  //  CExceptionHandler	m_ExceptionHandler;
  CInterruptSystem	m_Interrupt;
  CTimer			m_Timer;
  CLogger			m_Logger;
  CDWHCIDevice		m_DWHCI;
  
  CApp		m_App;
  static void MIDIPacketHandler (unsigned nCable, u8 *pPacket, unsigned nLength);
};

CKernel::CKernel(void):
  m_Screen(m_Options.GetWidth(), m_Options.GetHeight()),
  m_Timer(&m_Interrupt),
  m_Logger(m_Options.GetLogLevel (), &m_Timer),
  m_DWHCI(&m_Interrupt, &m_Timer),
  m_App(&m_Interrupt){
  m_ActLED.Blink (5);	// show we are alive
  engine_init();  
}

CKernel::~CKernel (void){}

CKernel* gKernel = 0; // make kernel globally accessible: MIDIPacketHandler has no void* payload field
void CKernel::MIDIPacketHandler (unsigned nCable, u8 *pPacket, unsigned nLength){
  if (nLength < 3) return;
  
  u8 ucStatus    = pPacket[0];
  //u8 ucChannel   = ucStatus & 0x0F;
  u8 ucType      = ucStatus >> 4;
  u8 ucKeyNumber = pPacket[1];
  //u8 ucVelocity  = pPacket[2];
  
  if (ucType == 0x9){
    //gKernel-> m_Logger.Write ("kernel", LogNotice, "MIDI ON");
    engine_on(ucKeyNumber);
  } else if (ucType == 0x8){
    //gKernel-> m_Logger.Write ("kernel", LogNotice, "MIDI OFF");
    engine_off(ucKeyNumber);
  }
}

boolean CKernel::Initialize (void){
  bool res = m_Screen.Initialize() 
    && m_Logger.Initialize(&m_Screen) 
    && m_Interrupt.Initialize() 
    && m_Timer.Initialize() 
    && m_DWHCI.Initialize();
  if (!res) return res;

  gKernel = this;
  CUSBMIDIDevice *pMIDIDevice =
    (CUSBMIDIDevice *) CDeviceNameService::Get()->GetDevice ("umidi1", FALSE);
  if (!pMIDIDevice){
    this->m_Logger.Write ("kernel", LogNotice, "No MIDI device!");
    return FALSE;
  }
  
  pMIDIDevice->RegisterPacketHandler(MIDIPacketHandler);
  return res;
}

int CKernel::Run (void){
  this->m_Logger.Write ("kernel", LogNotice, "Running!");
  this->m_App.Start();
  
  while (1){}
  return /*ShutdownHalt*/1;
}

int main (void){
  CKernel Kernel;
  if (!Kernel.Initialize ()){
    halt();
    return EXIT_HALT;
  }
  
  if (Kernel.Run() == /*ShutdownReboot*/2){
    reboot ();
    return EXIT_REBOOT;
  }
  halt ();
  return EXIT_HALT;
}
