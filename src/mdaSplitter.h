//See associated .cpp file for copyright and other info


#define NPARAMS  7  ///number of parameters
#define NPROGS   3  ///number of programs

#ifndef __mdaSplitter_H
#define __mdaSplitter_H

#include "audioeffectx.h"

class mdaSplitterProgram
{
public:
  mdaSplitterProgram()
  {
    param[0] = 0.10f;  //mode
    param[1] = 0.50f;  //freq
    param[2] = 0.25f;  //freq mode
    param[3] = 0.50f;  //level (was 2)
    param[4] = 0.50f;  //level mode
    param[5] = 0.50f;  //envelope
    param[6] = 0.50f;  //gain
    strcpy(name, "Frequency/Level Splitter");
  }
private:
  friend class mdaSplitter;
  float param[NPARAMS];
  char name[32];
};


class mdaSplitter : public AudioEffectX
{
public:
  mdaSplitter(audioMasterCallback audioMaster);

  virtual void  process(float **inputs, float **outputs, VstInt32 sampleFrames);
  virtual void  processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
  virtual void  setProgram(VstInt32 program);
  virtual void  setProgramName(char *name);
  virtual void  getProgramName(char *name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 which, char* name);
  virtual void  setParameter(VstInt32 which, float value);
  virtual float getParameter(VstInt32 which);
  virtual void  getParameterLabel(VstInt32 which, char *label);
  virtual void  getParameterDisplay(VstInt32 which, char *text);
  virtual void  getParameterName(VstInt32 which, char *text);
  virtual void  suspend();
  virtual void  resume();

	virtual bool getEffectName(char *name);
	virtual bool getVendorString(char *text);
	virtual bool getProductString(char *text);
	virtual VstInt32 getVendorVersion() { return 1000; }

protected:
  mdaSplitterProgram programs[NPROGS];

  ///global internal variables
  float freq, fdisp, buf0, buf1, buf2, buf3;  //filter
  float level, ldisp, env, att, rel;          //level switch
  float ff, ll, pp, i2l, i2r, o2l, o2r;       //routing (freq, level, phase, output)
  VstInt32  mode;

};

#endif
