//See associated .cpp file for copyright and other info


#define NPARAMS  7  ///number of parameters
#define NPROGS   3  ///number of programs

#ifndef __mdaSplitter_H
#define __mdaSplitter_H

#include "audioeffectx.h"

class mdaSplitterProgram
{
public:
  mdaSplitterProgram();
private:
  friend class mdaSplitter;
  float param[NPARAMS];
  char name[32];
};


class mdaSplitter : public AudioEffectX
{
public:
  mdaSplitter(audioMasterCallback audioMaster);
  ~mdaSplitter();

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
  mdaSplitterProgram *programs;

  ///global internal variables
  float freq, fdisp, buf0, buf1, buf2, buf3;  //filter
  float level, ldisp, env, att, rel;          //level switch
  float ff, ll, pp, i2l, i2r, o2l, o2r;       //routing (freq, level, phase, output)
  VstInt32  mode;

};

#endif
