// "mda Talkbox" v1.0  Copyright(c)2002 Paul Kellett (@mda-vst.com)
// Based on "SampleEffectUnit" © Copyright 2002 Apple Computer, Inc. All rights reserved.

#include "AUEffectBase.h"

#include "mda-common.h"


const float TWO_PI = 6.283185308f;
const float HALF_DEG2RAD = 0.0087266462611f;


//--------------------------------------------------------------------------------
enum
{
	kParam_Pan,
	kParam_Auto,
	kNumParams
};

const SInt16 kNumInputs = 1;
const SInt16 kNumOutputs = 2;


//--------------------------------------------------------------------------------
class RoundPan : public AUEffectBase
{
public:
	RoundPan(AudioUnit inComponentInstance);
	virtual ComponentResult Initialize();
	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual bool SupportsTail()
		{	return true;	}
	virtual UInt32 SupportedNumChannels(const AUChannelInfo ** outChannelInfo);
#if 0
	virtual void SetBypassEffect(bool inFlag);
#endif
	virtual ComponentResult Version()
		{	return PLUGIN_VERSION;	}
	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);

private:
	float l, r, oldpos, pos;
};



//--------------------------------------------------------------------------------
COMPONENT_ENTRY(RoundPan)


//--------------------------------------------------------------------------------
RoundPan::RoundPan(AudioUnit inComponentInstance) : AUEffectBase(inComponentInstance, false)
{
	l = r = oldpos = pos = 0.0f;

	// init internal parameters...
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AudioUnitParameterInfo paramInfo;
		ComponentResult result = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (result == noErr)
			SetParameter(i, paramInfo.defaultValue);
	}

/*
	CreateElements();
	CAStreamBasicDescription streamDesc = GetInput(0)->GetStreamFormat();
	streamDesc.SetCanonical(kNumInputs, false);
	GetInput(0)->SetStreamFormat(streamDesc);
*/
}

//--------------------------------------------------------------------------------
ComponentResult	RoundPan::Initialize()
{
	ComponentResult result = AUEffectBase::Initialize();

	if (result == noErr)
	{
		const AudioUnitElement elem = 0;

		Reset(kAudioUnitScope_Global, elem);

		if ( GetStreamFormat(kAudioUnitScope_Input, elem).mChannelsPerFrame != GetStreamFormat(kAudioUnitScope_Output, elem).mChannelsPerFrame )
		{
			if ( ProcessesInPlace() )
			{
				SetProcessesInPlace(false);
				PropertyChanged(kAudioUnitProperty_InPlaceProcessing, kAudioUnitScope_Global, elem);
			}
		}
	}

	return result;
}


//--------------------------------------------------------------------------------
ComponentResult RoundPan::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	ComponentResult result = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;
	
	switch(inParameterID)
	{
		case kParam_Pan:
			FillInParameterName(outParameterInfo, CFSTR("Pan"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Degrees;
			outParameterInfo.minValue = -180.0;
			outParameterInfo.maxValue = 180.0;
			outParameterInfo.defaultValue = 0.0;
			break;

		case kParam_Auto:
			FillInParameterName(outParameterInfo, CFSTR("Deg/sec"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Degrees;
			outParameterInfo.minValue = -720.0;
			outParameterInfo.maxValue = 720.0;
			outParameterInfo.defaultValue = 180.0;
			break;
			
		default:
			result = kAudioUnitErr_InvalidParameter;
			break;
	}
	return result;
}


//--------------------------------------------------------------------------------
UInt32 RoundPan::SupportedNumChannels(const AUChannelInfo ** outChannelInfo)
{
//	static AUChannelInfo plugChannelInfo[] = { {kNumInputs, kNumOutputs} };	// this plugin does mono->stereo only
	static AUChannelInfo plugChannelInfo[] = { {kNumInputs, kNumOutputs}, {kNumOutputs, kNumOutputs} };

	if (outChannelInfo != NULL)
		*outChannelInfo = plugChannelInfo;

	return sizeof(plugChannelInfo) / sizeof(plugChannelInfo[0]);
}

#if 0
//--------------------------------------------------------------------------------
void RoundPan::SetBypassEffect(bool inFlag)
{
	if ( GetInput(0)->GetStreamFormat().mChannelsPerFrame == GetOutput(0)->GetStreamFormat().mChannelsPerFrame )
		AUEffectBase::SetBypassEffect(inFlag);
}
#endif


//--------------------------------------------------------------------------------
OSStatus RoundPan::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags,
	const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
	const float * in = (float*)(inBuffer.mBuffers[0].mData);
	float * in2 = NULL;
	if (inBuffer.mNumberBuffers > 1)
		in2 = (float*)(inBuffer.mBuffers[1].mData);
	float * out[kNumOutputs];
	for (SInt16 i=0; i < kNumOutputs; i++)
		out[i] = (float*)(outBuffer.mBuffers[i].mData);

	// algorithm starts here...
	float fs = GetSampleRate();
	float dpos = HALF_DEG2RAD * GetParameter(kParam_Auto) / fs;

	float p = GetParameter(kParam_Pan);
	if (fabsf(p) < 2.0f)
		p = 0.0f;
	if (p != oldpos)
	{
		oldpos = p;
		pos = HALF_DEG2RAD * p;
	}

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		float inValue = in[samp];	// mono input
		if (in2 != NULL)
			inValue = (inValue + in2[samp]) * 0.5f;	// summed stereo input
		out[0][samp] = in[samp] * -sinf(pos - 0.7854f);	// could simplify & lerp!
		out[1][samp] = in[samp] *  sinf(pos + 0.7854f);

		pos += dpos;
	}
	if (pos < 0.0f)
		pos += TWO_PI;
	else if (pos > TWO_PI)
		pos -= TWO_PI;

	return noErr;
}
