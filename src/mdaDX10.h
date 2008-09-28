//See associated .cpp file for copyright and other info

#ifndef __mdaDX10__
#define __mdaDX10__

#include <string.h>

#include "audioeffectx.h"

#define NPARAMS 16       //number of parameters
#define NPROGS  32       //number of programs
#define NOUTS    2       //number of outputs
#define NVOICES  8       //max polyphony
#define SILENCE 0.0003f  //voice choking

class mdaDX10Program
{
  friend class mdaDX10;
private:
  float param[NPARAMS];
  char  name[24];
};


struct VOICE  //voice state
{
  float env;  //carrier envelope
  float dmod; //modulator oscillator 
  float mod0;
  float mod1;
  float menv; //modulator envelope
  float mlev; //modulator target level
  float mdec; //modulator envelope decay
  float car;  //carrier oscillator
  float dcar;
  float cenv; //smoothed env
  float catt; //smoothing
  float cdec; //carrier envelope decay
  VstInt32  note; //remember what note triggered this
};


class mdaDX10 : public AudioEffectX
{
public:
	mdaDX10(audioMasterCallback audioMaster);
	~mdaDX10();

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

	virtual bool getOutputProperties (VstInt32 index, VstPinProperties* properties);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);
	virtual bool copyProgram (VstInt32 destination);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion () {return 1;}
	virtual VstInt32 canDo (char* text);

	virtual VstInt32 getNumMidiInputChannels ()  { return 1; }

private:
	void update();  //my parameter update
  void noteOn(VstInt32 note, VstInt32 velocity);
  void fillpatch(VstInt32 p, char *name, 
                 float p0,  float p1,  float p2,  float p3,  float p4,  float p5, 
                 float p6,  float p7,  float p8,  float p9,  float p10, float p11, 
                 float p12, float p13, float p14, float p15);

  mdaDX10Program* programs;
  float Fs;

  #define EVENTBUFFER 120
  #define EVENTS_DONE 99999999
  VstInt32 notes[EVENTBUFFER + 8];  //list of delta|note|velocity for current block

  ///global internal variables
  VOICE voice[NVOICES];
  #define SUSTAIN 128
  VstInt32 sustain, activevoices, K;

  float tune, rati, ratf, ratio; //modulator ratio
  float catt, cdec, crel;        //carrier envelope
  float depth, dept2, mdec, mrel; //modulator envelope
  float lfo0, lfo1, dlfo, modwhl, MW, pbend, velsens, volume, vibrato; //LFO and CC
  float rich, modmix;
};

#endif
