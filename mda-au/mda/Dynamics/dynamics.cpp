// "mda Dynamics" v1.0  Copyright(c)2002 Paul Kellett (@mda-vst.com)

#include "AUEffectBase.h"

#include "mda-common.h"


const float TWOPI = 6.283185308f;


enum
{
	//_LIMIT,
	_COMP,
	_THR,
	_RAT,
	_ATT,	
	_REL,
	_GAIN,
	_GATE,
	_GTHR,
	_GATT,
	_GREL,
	NPARAM
};


class Dynamics : public AUEffectBase
{
public:
	Dynamics(AudioComponentInstance inComponentInstance);
	virtual OSStatus Initialize();
	virtual void Cleanup();
	virtual OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual bool SupportsTail() { return true; }
	virtual OSStatus Version() { return PLUGIN_VERSION; }
	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);

private:
	float env, env2, genv;
	float **in;
	float **out;
};


COMPONENT_ENTRY(Dynamics)


Dynamics::Dynamics(AudioComponentInstance inComponentInstance) : AUEffectBase(inComponentInstance)
{
	env = env2 = genv = 0.0f;

	in = out = NULL;

	// init internal parameters...
	for (AudioUnitParameterID i=0; i < NPARAM; i++)
	{
		AudioUnitParameterInfo paramInfo;
		OSStatus status = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (status == noErr)
			SetParameter(i, paramInfo.defaultValue);
	}
}


OSStatus Dynamics::Initialize()
{
	OSStatus status = AUEffectBase::Initialize();

	if (status == noErr)
	{
		in = (float**) malloc(GetNumberOfChannels() * sizeof(float*));
		out = (float**) malloc(GetNumberOfChannels() * sizeof(float*));
		if ( (in == NULL) || (out == NULL) )
			status = memFullErr;
	}

	return status;
}

void Dynamics::Cleanup()
{
	AUEffectBase::Cleanup();

	if (in != NULL)
		free(in);
	in = NULL;
	if (out != NULL)
		free(out);
	out = NULL;
}


OSStatus Dynamics::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	OSStatus status = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
/*		case _LIMIT:
			FillInParameterName(outParameterInfo, CFSTR("Limiter"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -20.0;
			outParameterInfo.maxValue = 0.0;
			outParameterInfo.defaultValue = 0.0;
			break;
*/
		case _COMP:
			FillInParameterName(outParameterInfo, CFSTR("Compressor"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
			outParameterInfo.minValue = 0.0;
			outParameterInfo.maxValue = 1.0;
			outParameterInfo.defaultValue = 1.0;
			break;
			
		case _THR:
			FillInParameterName(outParameterInfo, CFSTR("Threshold"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -40.0;
			outParameterInfo.maxValue = 0.0;
			outParameterInfo.defaultValue = -18.0;
			break;

		case _RAT:
			FillInParameterName(outParameterInfo, CFSTR("Ratio"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Ratio;
			outParameterInfo.minValue = 1.0;
			outParameterInfo.maxValue = 20.0;
			outParameterInfo.defaultValue = 8.0;
			break;

		case _ATT:
			FillInParameterName(outParameterInfo, CFSTR("Attack"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
			outParameterInfo.minValue = 0.0001f;
			outParameterInfo.maxValue = 0.02f;
			outParameterInfo.defaultValue = 0.01f;
			break;

		case _REL:
			FillInParameterName(outParameterInfo, CFSTR("Release"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
			outParameterInfo.minValue = 0.02f;
			outParameterInfo.maxValue = 0.5f;
			outParameterInfo.defaultValue = 0.1f;
			break;

		case _GAIN:
			FillInParameterName(outParameterInfo, CFSTR("Output"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -20.0;
			outParameterInfo.maxValue = 20.0;
			outParameterInfo.defaultValue = 6.0;
			break;

		case _GATE:
			FillInParameterName(outParameterInfo, CFSTR("Gate"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
			outParameterInfo.minValue = 0;
			outParameterInfo.maxValue = 1;
			outParameterInfo.defaultValue = 0;
			break;

		case _GTHR:
			FillInParameterName(outParameterInfo, CFSTR("Threshold"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -80.0;
			outParameterInfo.maxValue = 0.0;
			outParameterInfo.defaultValue = -40.0;
			break;

		case _GATT:
			FillInParameterName(outParameterInfo, CFSTR("Attack"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
			outParameterInfo.minValue = 0.0001f;
			outParameterInfo.maxValue = 0.1f;
			outParameterInfo.defaultValue = 0.001f;
			break;

		case _GREL:
			FillInParameterName(outParameterInfo, CFSTR("Release"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
			outParameterInfo.minValue = 0.001f;
			outParameterInfo.maxValue = 4.0f;
			outParameterInfo.defaultValue = 0.1f;
			break;
			
		default:
			status = kAudioUnitErr_InvalidParameter;
			break;
	}
	return status;
}


OSStatus Dynamics::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags,
	const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
	UInt32 nChannels = outBuffer.mNumberBuffers;

	for(UInt32 i=0; i<nChannels; i++)
	{
		in[i] = (float*)inBuffer.mBuffers[i].mData;
		out[i] = (float*)outBuffer.mBuffers[i].mData;
	}
	
  ///////algorithm starts here...
  float lth, th, ra, at, re, tr, gth, ga, gr, fs = GetSampleRate();

  lth = 1.0f; //(float)pow(10.0f, 0.05f * GetParameter(_LIMIT));

	if((long)GetParameter(_COMP))
	{
    th = (float)pow(10.0f, 0.05f * GetParameter(_THR));
		ra = 0.06f * GetParameter(_RAT) - 0.06f;
		tr = (float)pow(10.0f, 0.05f * GetParameter(_GAIN));
	}
	else
	{
		th = 10.0f;
		ra = 0.0f;
		tr = 1.0f;
	}
	at = 1.0f - (float)exp(-1.0f / (fs * GetParameter(_ATT))); if(at > 1.0f) at = 1.0f;
	re = (float)exp(-1.0f / (fs * GetParameter(_REL))); if(re > 1.0f) re = 1.0f;
	
  if((long)GetParameter(_GATE))
	{
    gth = (float)pow(10.0f, 0.05f * GetParameter(_GTHR));
	}
	else
	{
		gth = -1.0f;
	}
	ga = 1.0f - (float)exp(-5.0f / (fs * GetParameter(_GATT))); if(ga > 1.0f) ga = 1.0f; 
	gr = (float)exp(-5.0f / (fs * GetParameter(_GREL))); if(gr > 1.0f) gr = 1.0f;
 

	UInt32 c;
	float i, y, g, e=env, e2=env2, ge=genv;

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		i = 0.0f;
		for(c=0; c<nChannels; c++) { y = fabsf(in[c][samp]); if(y > i) i = y; } //peak detector

		e *= re;
		e2 *= re;
		if(i > e) { e += at * (i - e);  e2 = i; }
		g = (e > th)? tr / (1.0f + ra * ((e / th) - 1.0f)) : tr; //compress

		//if(g < 0.0f) g = 0.0f;
		if(g * e2 > lth) g = lth / e2; //limit 

		if(e > gth) ge += ga - ga * ge; else ge *= gr; //gate
		g *= ge;
		
		for(c=0; c<nChannels; c++) //apply gain
		{
			y = in[c][samp] * g;
			out[c][samp] = y;
    }
  }
  env  = (e  < 0.000001f)? 0.0f : e; //save envelopes & anti-denormal
  env2 = (e2 < 0.000001f)? 0.0f : e2;
  genv = (ge < 0.000001f)? 0.0f : ge;
	
	return noErr;
}


/* how to use ioSilence????

OSStatus	AUEffectBase::ProcessBufferLists(AudioUnitRenderActionFlags &	ioActionFlags,
{

	bool silence = (ioActionFlags & kAudioUnitRenderAction_OutputIsSilence) != 0;
 //silence = output is slience

	ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
//un-flag output is silence
	
	bool ioSilence;

	ioSilence = silentInput;

	kernel->Process(ioSilence);

	if(!ioSilence) ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;
//if not iosilence,  
}
*/
