#ifndef __mdaOverdrive_H
#define __mdaOverdrive_H

#include "audioeffectx.h"

class mdaOverdrive : public AudioEffectX
{
public:
	mdaOverdrive(audioMasterCallback audioMaster);
	~mdaOverdrive();

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
	float fParam1;
  float fParam2;
  float fParam3;

  float filt1, filt2; //filter buffers
  float filt;         //filter coeff.
  float gain;         //output gain

	char programName[32];
};

#endif
