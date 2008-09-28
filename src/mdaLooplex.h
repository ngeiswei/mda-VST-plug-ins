//See associated .cpp file for copyright and other info

#ifndef __mdaLooplex__
#define __mdaLooplex__

#include <string.h>

#include "audioeffectx.h"

#define NPARAMS  7       //number of parameters
#define NPROGS   1       //number of programs
#define NOUTS    2       //number of outputs
#define SILENCE  0.00001f


class mdaLooplexProgram
{
  friend class mdaLooplex;
public:
	mdaLooplexProgram();
private:
  float param[NPARAMS];
  char  name[24];
};


class mdaLooplex : public AudioEffectX
{
public:
	mdaLooplex(audioMasterCallback audioMaster);
	~mdaLooplex();

	virtual void process(float **inputs, float **outputs, VstInt32 sampleframes);
	virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleframes);
	virtual VstInt32 processEvents(VstEvents* events);

	virtual void setProgram(VstInt32 program);
	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual void setParameter(VstInt32 index, float value);
	virtual float getParameter(VstInt32 index);
	virtual void getParameterLabel(VstInt32 index, char *label);
	virtual void getParameterDisplay(VstInt32 index, char *text);
	virtual void getParameterName(VstInt32 index, char *text);
	virtual void setSampleRate(float sampleRate);
	virtual void setBlockSize(VstInt32 blockSize);
    virtual void resume();

	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);
	virtual bool copyProgram (VstInt32 destination);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion () {return 1;}
	virtual VstInt32 canDo (char* text);

	virtual VstInt32 getNumMidiInputChannels ()  { return 1; }
	
  void idle();
  
private:
	void update();  //my parameter update

  mdaLooplexProgram* programs;
  float Fs;

  #define EVENTBUFFER 120
  #define EVENTS_DONE 99999999
  VstInt32 notes[EVENTBUFFER + 8];  //list of delta|note|velocity for current block

  ///global internal variables
  float in_mix, in_pan, out_mix, feedback, modwhl;

  short *buffer;
  VstInt32 bufpos, buflen, bufmax, mode;

  VstInt32 bypass, bypassed, busy, status, recreq;
  float oldParam0, oldParam1, oldParam2;

};

#endif
