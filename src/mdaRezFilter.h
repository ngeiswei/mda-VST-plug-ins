#ifndef __mdaRezFilter_H
#define __mdaRezFilter_H

#include "audioeffectx.h"

class mdaRezFilter : public AudioEffectX
{
public:
	mdaRezFilter(audioMasterCallback audioMaster);
	~mdaRezFilter();

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
  float fParam5;
  float fParam6;
  float fParam7;
  float fParam8;
  float fParam9;

  float fff, fq, fg, fmax;
  float env, fenv, att, rel;
  float flfo, phi, dphi, bufl;
  float buf0, buf1, buf2, tthr, env2;
  int lfomode, ttrig, tatt;

  char bugFix[32]; //Program name was corrupted here!
  char programName[32];
};

#endif
