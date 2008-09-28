//See associated .cpp file for copyright and other info


#define NPARAMS  3  ///number of parameters
#define NPROGS   8  ///number of programs

#ifndef __mdaLoudness_H
#define __mdaLoudness_H

#include "audioeffectx.h"

class mdaLoudnessProgram
{
public:
  mdaLoudnessProgram();
private:
  friend class mdaLoudness;
  float param[NPARAMS];
  char name[32];
};


class mdaLoudness : public AudioEffectX
{
public:
  mdaLoudness(audioMasterCallback audioMaster);
  ~mdaLoudness();

  virtual void  process(float **inputs, float **outputs, VstInt32 sampleFrames);
  virtual void  processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
  virtual void  setProgram(VstInt32 program);
  virtual void  setProgramName(char *name);
  virtual void  getProgramName(char *name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* name);
  virtual void  setParameter(VstInt32 index, float value);
  virtual float getParameter(VstInt32 index);
  virtual void  getParameterLabel(VstInt32 index, char *label);
  virtual void  getParameterDisplay(VstInt32 index, char *text);
  virtual void  getParameterName(VstInt32 index, char *text);
  virtual void  suspend();
  virtual void  resume();

	virtual bool getEffectName(char *name);
	virtual bool getVendorString(char *text);
	virtual bool getProductString(char *text);
	virtual VstInt32 getVendorVersion() { return 1000; }

protected:
  mdaLoudnessProgram *programs;

  ///global internal variables
  float Z0, Z1, Z2, Z3, A0, A1, A2, gain;
  float igain, ogain;
  VstInt32  mode;
};

#endif
