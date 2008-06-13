#include "AUEffectBase.h"

#include "mda-common.h"


//--------------------------------------------------------------------------------
enum {
	kParam_Headroom = 0,
	kParam_BitQuantize,
	kParam_SampleRate,
	kParam_SampleRateMode,
	kParam_PostFilter,
	kParam_NonLinearity,
	kParam_NonLinearityMode,
	kParam_Output,
	kNumParams
};

enum {
	kSampleRateMode_Sample = 0,
	kSampleRateMode_SampleAndHold,
	kNumSampleRateModes
};

enum {
	kNonLinearityMode_Odd = 0,
	kNonLinearityMode_Even,
	kNumNonLinearityModes
};


//--------------------------------------------------------------------------------
class Degrade : public AUEffectBase
{
public:
	Degrade(AudioUnit inComponentInstance);

	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual ComponentResult GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
	virtual bool SupportsTail()
		{	return true;	}
	virtual ComponentResult Version()
		{	return PLUGIN_VERSION;	}
	virtual AUKernelBase * NewKernel()
		{	return new DegradeKernel(this);	}

private:
	class DegradeKernel : public AUKernelBase
	{
	public:
		DegradeKernel(AUEffectBase * inAudioUnit);
		virtual void Reset();
		virtual void Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool & ioSilence);

	private:
		float filterFreq(float hz);

		float buf0, buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf9;
		long tcount;
	};
};



#pragma mark -

//--------------------------------------------------------------------------------
COMPONENT_ENTRY(Degrade)

//--------------------------------------------------------------------------------
Degrade::Degrade(AudioUnit inComponentInstance)
	: AUEffectBase(inComponentInstance)
{
	// make sure that GetSampleRate() is available to be called without crashing
	CreateElements();

	// init internal parameters...
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AudioUnitParameterInfo paramInfo;
		ComponentResult result = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (result == noErr)
			SetParameter(i, paramInfo.defaultValue);
	}
}

//--------------------------------------------------------------------------------
Degrade::DegradeKernel::DegradeKernel(AUEffectBase * inAudioUnit)
	: AUKernelBase(inAudioUnit)
{
	Reset();
}

//--------------------------------------------------------------------------------
void Degrade::DegradeKernel::Reset()
{
	buf0 = buf1 = buf2 = buf3 = buf4 = buf5 = buf6 = buf7 = buf8 = buf9 = 0.0f;
	tcount = 1;
}

//--------------------------------------------------------------------------------
ComponentResult Degrade::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	ComponentResult result = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_Headroom:
			FillInParameterName(outParameterInfo, CFSTR("headroom"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -30.0f;
			outParameterInfo.maxValue = 0.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_BitQuantize:
			FillInParameterName(outParameterInfo, CFSTR("quantize"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_CustomUnit;
			outParameterInfo.unitName = CFSTR("bits");
			outParameterInfo.minValue = 4;
			outParameterInfo.maxValue = 16;
			outParameterInfo.defaultValue = 10;
			break;

		case kParam_SampleRate:
			FillInParameterName(outParameterInfo, CFSTR("sample rate"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Rate;
#if 0
//GetSampleRate()/tn
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 9.0f;
			outParameterInfo.defaultValue = 2.7f;
#elif 0
			outParameterInfo.minValue = 1.0f;
			outParameterInfo.maxValue = 8103.0f;
			outParameterInfo.defaultValue = 15.0f;
//			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayExponential;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayLogarithmic;
#else
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = 20.0f;
			outParameterInfo.maxValue = GetSampleRate();
			outParameterInfo.defaultValue = 3000.0f;
//			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayExponential;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayCubeRoot;
			SetParamHasSampleRateDependency(true);
#endif
			break;

		case kParam_SampleRateMode:
			FillInParameterName(outParameterInfo, CFSTR("sample rate mode"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0;
			outParameterInfo.maxValue = kNumSampleRateModes - 1;
			outParameterInfo.defaultValue = kSampleRateMode_SampleAndHold;
			break;

		case kParam_PostFilter:
			FillInParameterName(outParameterInfo, CFSTR("post filter"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = 200.0f;
			outParameterInfo.maxValue = 20000.0f;
			outParameterInfo.defaultValue = 12600.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayLogarithmic;
			break;

		case kParam_NonLinearity:
			FillInParameterName(outParameterInfo, CFSTR("non-linearity"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 16.0f;
			break;

		case kParam_NonLinearityMode:
			FillInParameterName(outParameterInfo, CFSTR("non-linearity mode"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0;
			outParameterInfo.maxValue = kNumNonLinearityModes - 1;
			outParameterInfo.defaultValue = kNonLinearityMode_Even;
			break;

		case kParam_Output:
			FillInParameterName(outParameterInfo, CFSTR("output"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -20.0f;
			outParameterInfo.maxValue = 20.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		default:
			result = kAudioUnitErr_InvalidParameter;
			break;
	}

	return result;
}

//--------------------------------------------------------------------------------
ComponentResult	Degrade::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_SampleRateMode:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, CFSTR("S|S&&H"), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;

		case kParam_NonLinearityMode:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, CFSTR("odd|even"), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}

//--------------------------------------------------------------------------------
float Degrade::DegradeKernel::filterFreq(float hz)
{
	float j, k, r=0.999f;
	
	j = r * r - 1;
	k = 2.0f - 2.0f * r * r * cosf(0.647f * hz / (float)GetSampleRate() );
	return (sqrtf(k*k - 4.0f*j*j) - k) / (2.0f*j);
}

//--------------------------------------------------------------------------------
void Degrade::DegradeKernel::Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32, bool & ioSilence)
{
	// parameter-change calcs here
#if 0
	long tn = (long) expf(GetParameter(kParam_SampleRate));
#elif 0
	long tn = (long) GetParameter(kParam_SampleRate);
#else
	long tn = (long) ( GetSampleRate() / GetParameter(kParam_SampleRate) );
#endif
	if (tn < 1)
		tn = 1;
	long sampleRateMode = (long) GetParameter(kParam_SampleRateMode);
	float mode;
	if (sampleRateMode == kSampleRateMode_SampleAndHold)
		mode = 1.0f;
 	else
		mode = 0.0f;
		
//	tcount = 1;	// XXX when does this need to happen?
	float clp = powf(10.0f, GetParameter(kParam_Headroom)/20.0f);
	float fo2 = filterFreq( GetParameter(kParam_PostFilter) );
	float fi2 = (1.0f-fo2); fi2 = fi2*fi2; fi2 = fi2*fi2;
	float gi = powf(2.0f, (long)GetParameter(kParam_BitQuantize) - 2);	// XXX why is this 2 - 14 instead of 4 - 16?
	float go = 1.0f / gi;
	if (sampleRateMode == kSampleRateMode_SampleAndHold)
		gi = -gi/(float)tn;
	else
		gi = -gi;
	float ga = powf(10.0f, GetParameter(kParam_Output)/20.0f);
	float lin, lin2;
	lin = powf(10.0f, GetParameter(kParam_NonLinearity) * -0.003f);
	long nonLinearityMode = (long) GetParameter(kParam_NonLinearityMode);
	if (nonLinearityMode == kNonLinearityMode_Even)
		lin2 = lin;
	else
		lin2 = 1.0f;



	float b0=buf0;
	float b1=buf1, b2=buf2, b3=buf3, b4=buf4, b5=buf5;
	float b6=buf6, b7=buf7, b8=buf8, b9=buf9;
	long t = tcount; 	

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		b0 = inSourceP[samp] + mode * b0;

		if (t >= tn)
		{
			t = 0;
			b5 = (float)(go * (long)(b0 * gi));
			if (b5 > 0.0f)
			{
				b5 = powf(b5, lin2);
				if (b5 > clp)
					b5 = clp;
			}
			else
			{
				b5 = -powf(-b5, lin);
				if (b5 < -clp)
					b5 = -clp;
			}
			b0 = 0.0f;
		} 
		t++;

		b1 = fi2 * (b5 * ga) + fo2 * b1;
		b2 = b1 + fo2 * b2;
		b3 = b2 + fo2 * b3;
		b4 = b3 + fo2 * b4;
		b6 = fi2 * b4 + fo2 * b6;
		b7 = b6 + fo2 * b7;
		b8 = b7 + fo2 * b8;
		b9 = b8 + fo2 * b9;

		inDestP[samp] = b9;
	}
	if (fabsf(b1) < 1.0e-10f)
	{
		buf1 = 0.0f;
		buf2 = 0.0f;
		buf3 = 0.0f;
		buf4 = 0.0f;
		buf6 = 0.0f;
		buf7 = 0.0f;
		buf8 = 0.0f;
		buf9 = 0.0f;
		buf0 = 0.0f;
		buf5 = 0.0f;
	}
	else 
	{
		buf1 = b1;
		buf2 = b2;
		buf3 = b3;
		buf4 = b4;
		buf6 = b6;
		buf7 = b7;
		buf8 = b8;
		buf9 = b9;
		buf0 = b0;
		buf5 = b5;
		tcount = t;
	}
}
