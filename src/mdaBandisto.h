#ifndef __mdaBandisto_H
#define __mdaBandisto_H

#include "audioeffectx.h"

class mdaBandisto : public AudioEffectX
{
public:
	mdaBandisto(audioMasterCallback audioMaster);
	~mdaBandisto();

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

	virtual bool getEffectName(char *name);
	virtual bool getVendorString(char *text);
	virtual bool getProductString(char *text);
	virtual VstInt32 getVendorVersion() { return 1000; }

protected:
	float fParam1, fParam2, fParam3, fParam4;
	float fParam5, fParam6, fParam7, fParam8;
  float fParam9, fParam10;
  float gain1, driv1, trim1;
  float gain2, driv2, trim2;
  float gain3, driv3, trim3;
  float fi1, fb1, fo1, fi2, fb2, fo2, fb3, slev;
  int valve;
	char programName[32];
};

#endif
