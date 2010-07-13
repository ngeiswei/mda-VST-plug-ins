#include "AUEffectBase.h"

#include "mda-common.h"


//--------------------------------------------------------------------------------
enum {
	kParam_Freq = 0,
	kParam_Fine,
	kParam_Feedback,
	kNumParams
};

const float kTwoPi = 6.2831853f;


//--------------------------------------------------------------------------------
class Ring : public AUEffectBase
{
public:
	Ring(AudioComponentInstance inComponentInstance);

	virtual OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual bool SupportsTail()
		{	return true;	}
	virtual OSStatus Version()
		{	return PLUGIN_VERSION;	}
	virtual AUKernelBase * NewKernel()
		{	return new RingKernel(this);	}

private:
	class RingKernel : public AUKernelBase
	{
	public:
		RingKernel(AUEffectBase * inAudioUnit);
		virtual void Reset();
		virtual void Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool & ioSilence);

	private:
		float fPhi;
		float fprev;
	};
};



#pragma mark -

//--------------------------------------------------------------------------------
COMPONENT_ENTRY(Ring)

//--------------------------------------------------------------------------------
Ring::Ring(AudioComponentInstance inComponentInstance)
	: AUEffectBase(inComponentInstance)
{
	// init internal parameters...
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AudioUnitParameterInfo paramInfo;
		OSStatus status = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (status == noErr)
			SetParameter(i, paramInfo.defaultValue);
	}
}

//--------------------------------------------------------------------------------
Ring::RingKernel::RingKernel(AUEffectBase * inAudioUnit)
	: AUKernelBase(inAudioUnit)
{
	Reset();
}

//--------------------------------------------------------------------------------
void Ring::RingKernel::Reset()
{
	fPhi = 0.0f;
	fprev = 0.0f;
}

//--------------------------------------------------------------------------------
OSStatus Ring::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	OSStatus status = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_Freq:
			FillInParameterName(outParameterInfo, CFSTR("frequency"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = 0.1f;
			outParameterInfo.maxValue = 16000.0f;
			outParameterInfo.defaultValue = 1000.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplaySquareRoot;
			break;

		case kParam_Fine:
			FillInParameterName(outParameterInfo, CFSTR("fine tune"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_Feedback:
			FillInParameterName(outParameterInfo, CFSTR("feedback"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 95.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		default:
			status = kAudioUnitErr_InvalidParameter;
			break;
	}

	return status;
}

//--------------------------------------------------------------------------------
void Ring::RingKernel::Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32, bool & ioSilence)
{
	// parameter-change calcs here
	float fdPhi = (kTwoPi * 100.0f * (GetParameter(kParam_Fine)*0.01f + GetParameter(kParam_Freq)*0.01f) / (float)GetSampleRate());
	float ffb = GetParameter(kParam_Feedback) * 0.01f;

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		float g = sinf(fPhi);	// instantaneous gain
		fPhi = fmodf( fPhi + fdPhi, kTwoPi );	// oscillator phase

		fprev  = (ffb * fprev + inSourceP[samp]) * g;

		inDestP[samp] = fprev;	// ...and store operations at the end, as this uses the cache efficiently.
	}
}
