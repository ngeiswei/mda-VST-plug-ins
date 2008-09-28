#ifndef __mdaAmbience_H
#define __mdaAmbience_H

#include "audioeffectx.h"

class mdaAmbience : public AudioEffectX
{
public:
	mdaAmbience(audioMasterCallback audioMaster);
	~mdaAmbience();

	virtual void process(float **inputs, float **outputs, VstInt32 sampleFrames);
	virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* name);
	virtual void setParameter(VstInt32 index, float value);
	virtual float getParameter(VstInt32 index);
	virtual void getParameterLabel(VstInt32 index, char *label);
	virtual void getParameterDisplay(VstInt32 index, char *text);
	virtual void getParameterName(VstInt32 index, char *text);
  virtual void suspend();

	virtual bool getEffectName(char *name);
	virtual bool getVendorString(char *text);
	virtual bool getProductString(char *text);
	virtual VstInt32 getVendorVersion() { return 1000; }

protected:
	float fParam0;
  float fParam1;
  float fParam2;
  float fParam3;
  float fParam4;

  float *buf1, *buf2, *buf3, *buf4;
  float fil, fbak, damp, wet, dry, size;
  VstInt32  pos, den, rdy;

  char programName[32];
};

#endif
