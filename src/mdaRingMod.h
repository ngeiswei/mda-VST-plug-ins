#ifndef __mdaRingMod_H
#define __mdaRingMod_H

#include "audioeffectx.h"

class mdaRingMod : public AudioEffectX
{
public:
	mdaRingMod(audioMasterCallback audioMaster);
	~mdaRingMod();

	virtual void process(float **inputs, float **outputs, VstInt32 sampleFrames);
	virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual void setParameter(VstInt32 index, float value);
	virtual float getParameter(VstInt32 index);
	virtual void getParameterLabel(VstInt32 index, char *label);
	virtual void getParameterDisplay(VstInt32 index, char *text);
	virtual void getParameterName(VstInt32 index, char *text);

	virtual bool getEffectName(char *name);
	virtual bool getVendorString(char *text);
	virtual bool getProductString(char *text);
	virtual VstInt32 getVendorVersion() { return 1000; }

protected:
	float fParam1;
  float fParam2;
  float fParam3;
  float fParam4;
  float fPhi;
  float fdPhi;
  float nul;
  float twoPi;
  float ffb, fprev;

	char programName[32];
};

#endif
