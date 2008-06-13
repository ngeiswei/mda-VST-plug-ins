#ifndef __mdaTransient_H
#define __mdaTransient_H

#include "audioeffectx.h"

class mdaTransient : public AudioEffectX
{
public:
	mdaTransient(audioMasterCallback audioMaster);
	~mdaTransient();

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
  float fParam5;
  float fParam6;
  float dry, att1, att2, rel12, att34, rel3, rel4;
  float env1, env2, env3, env4, fili, filo, filx, fbuf1, fbuf2;

	char programName[32];
};

#endif
