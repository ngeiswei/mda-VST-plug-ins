#ifndef __mdaRePsycho_H
#define __mdaRePsycho_H

#include "audioeffectx.h"

class mdaRePsycho : public AudioEffectX
{
public:
	mdaRePsycho(audioMasterCallback audioMaster);
	~mdaRePsycho();

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
	float fParam1;
  float fParam2;
  float fParam3;
  float fParam4;
  float fParam5;
  float fParam6;
  float fParam7;
  float thr, env, gai, tun, wet, dry, fil, buf, buf2;
  VstInt32 tim, dtim;

  float *buffer, *buffer2;
	VstInt32 size;

	char programName[32];
};

#endif
