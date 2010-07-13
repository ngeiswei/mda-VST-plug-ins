#include "AUEffectBase.h"

#include "mda-common.h"


//--------------------------------------------------------------------------------
enum
{
	kParam_Type,
	kParam_Level,
	kParam_Tune,
	kParam_DryMix,
	kParam_Threshold,
	kParam_Release,
	kNumParams
};

enum
{
	kMode_Distort,
	kMode_Divide,
	kMode_Invert,
	kMode_KeyOsc,
	kNumModes
};



//--------------------------------------------------------------------------------
class SubSynth : public AUEffectBase
{
public:
	SubSynth(AudioComponentInstance inComponentInstance);

	virtual OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual OSStatus GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
	virtual OSStatus GetPropertyInfo(AudioUnitPropertyID inPropertyID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32 & outDataSize, Boolean & outWritable);
	virtual OSStatus GetProperty(AudioUnitPropertyID inPropertyID, AudioUnitScope inScope, AudioUnitElement inElement, void * outData);

	virtual bool SupportsTail()
		{	return true;	}
	virtual Float64 GetTailTime();
	virtual OSStatus Version()
		{	return PLUGIN_VERSION;	}
	virtual AUKernelBase * NewKernel()
		{	return new SubSynthKernel(this);	}


private:
	double GetReleaseTimeForParamValue(double inLinearValue);

	class SubSynthKernel : public AUKernelBase
	{
	public:
		SubSynthKernel(AUEffectBase * inAudioUnit);
		virtual void Reset();
		virtual void Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool & ioSilence);

	private:
		Float32 GetParameter_scalar(AudioUnitParameterID inParameterID);

		float filt1, filt2, filt3, filt4;
		float env, phi;
	};
};



#pragma mark -

//--------------------------------------------------------------------------------
COMPONENT_ENTRY(SubSynth)

//--------------------------------------------------------------------------------
SubSynth::SubSynth(AudioComponentInstance inComponentInstance)
	: AUEffectBase(inComponentInstance)
{
	// init internal parameters...
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AudioUnitParameterInfo paramInfo;
		OSStatus status = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (status == noErr)
			AUEffectBase::SetParameter(i, paramInfo.defaultValue);
	}
}

//--------------------------------------------------------------------------------
SubSynth::SubSynthKernel::SubSynthKernel(AUEffectBase * inAudioUnit)
	: AUKernelBase(inAudioUnit)
{
	Reset();
}

//--------------------------------------------------------------------------------
void SubSynth::SubSynthKernel::Reset()
{
	phi = env = filt1 = filt2 = filt3 = filt4 = 0.0f;
}

//--------------------------------------------------------------------------------
Float64 SubSynth::GetTailTime()
{
	return GetReleaseTimeForParamValue(1.0) * 0.001;
}



#pragma mark -
#pragma mark parameters

//--------------------------------------------------------------------------------
OSStatus SubSynth::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	OSStatus status = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_Type:
			FillInParameterName(outParameterInfo, CFSTR("Type"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = kNumModes - 1;
			outParameterInfo.defaultValue = kMode_Distort;
			break;

		case kParam_Level:
			FillInParameterName(outParameterInfo, CFSTR("Level"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 30.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayCubeRoot;
			break;

//0.0726f * GetSampleRate() * powf(10.0f, -2.5f + (1.5f * fParam3))	// 10 - 320 Hz
		case kParam_Tune:
			FillInParameterName(outParameterInfo, CFSTR("Tune"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 1.0f;
			outParameterInfo.defaultValue = 0.6f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_ValuesHaveStrings;
			break;

		case kParam_DryMix:
			FillInParameterName(outParameterInfo, CFSTR("Dry Mix"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 100.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayCubeRoot;
			break;

		case kParam_Threshold:
			FillInParameterName(outParameterInfo, CFSTR("Threshold"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -60.0f;
			outParameterInfo.maxValue = 0.0f;
			outParameterInfo.defaultValue = -24.0f;
			break;

//rls = 1.0f - powf(10.0f, -2.0f - (3.0f * fParam6))
//-301.03f / (GetSampleRate() * log10f(rls))	// 1 - 1569 ms
		case kParam_Release:
			FillInParameterName(outParameterInfo, CFSTR("Release"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Milliseconds;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 1.0f;
			outParameterInfo.defaultValue = 0.65f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_ValuesHaveStrings;
			break;

		default:
			status = kAudioUnitErr_InvalidParameter;
			break;
	}

	return status;
}

//--------------------------------------------------------------------------------
OSStatus SubSynth::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_Type:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("distort|divide|invert|key osc."), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}

//--------------------------------------------------------------------------------
Float32 SubSynth::SubSynthKernel::GetParameter_scalar(AudioUnitParameterID inParameterID)
{
	return (GetParameter(inParameterID) * 0.01f);
}

//--------------------------------------------------------------------------------
double SubSynth::GetReleaseTimeForParamValue(double inLinearValue)
{
	inLinearValue = 1.0 - pow(10.0, -2.0 - (3.0 * inLinearValue));
	if (inLinearValue == 0.0)
		inLinearValue = 10.0;	// XXX avoid log10(0) and division by 0
	return -301.03 / (GetSampleRate() * log10(fabs(inLinearValue)));
}

//--------------------------------------------------------------------------------
OSStatus SubSynth::GetPropertyInfo(AudioUnitPropertyID inPropertyID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32 & outDataSize, Boolean & outWritable)
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inPropertyID)
		{
			case kAudioUnitProperty_ParameterStringFromValue:
				outWritable = false;
				outDataSize = sizeof(AudioUnitParameterStringFromValue);
				return noErr;

			case kAudioUnitProperty_ParameterValueFromString:
				outWritable = false;
				outDataSize = sizeof(AudioUnitParameterValueFromString);
				return noErr;
		}
	}

	return AUEffectBase::GetPropertyInfo(inPropertyID, inScope, inElement, outDataSize, outWritable);
}

//--------------------------------------------------------------------------------
OSStatus SubSynth::GetProperty(AudioUnitPropertyID inPropertyID, AudioUnitScope inScope, AudioUnitElement inElement, void * outData)
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inPropertyID)
		{
			case kAudioUnitProperty_ParameterValueFromString:
			{
				AudioUnitParameterValueFromString * name = (AudioUnitParameterValueFromString*)outData;
				if (name->inString == NULL)
					return kAudioUnitErr_InvalidPropertyValue;
				double paramValue_literal = CFStringGetDoubleValue(name->inString);
				switch (name->inParamID)
				{
					case kParam_Tune:
						if (paramValue_literal <= 0.0)
							name->outValue = 0.0;	// XXX avoid log10(0) or log10(-X)
						else
							name->outValue = (log10(paramValue_literal / (0.0726 * GetSampleRate())) + 2.5) / 1.5;
						break;
					case kParam_Release:
						return kAudioUnitErr_PropertyNotInUse;	// XXX I can't figure out how to invert this one
					default:
						return kAudioUnitErr_InvalidParameter;
				}
				return noErr;
			}

			case kAudioUnitProperty_ParameterStringFromValue:
			{
				AudioUnitParameterStringFromValue * name = (AudioUnitParameterStringFromValue*)outData;
				double paramValue = (name->inValue == NULL) ? GetParameter(name->inParamID) : *(name->inValue);
				int precision = 0;
				switch (name->inParamID)
				{
					case kParam_Tune:
						paramValue = 0.0726 * GetSampleRate() * pow(10.0, -2.5 + (1.5 * paramValue));
						precision = 3;
						break;
					case kParam_Release:
						paramValue = GetReleaseTimeForParamValue(paramValue);
						precision = 1;
						break;
					default:
						return kAudioUnitErr_InvalidParameter;
				}
				name->outString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%.*f"), precision, paramValue);
				if (name->outString == NULL)
					return coreFoundationUnknownErr;
				return noErr;
			}
		}
	}

	return AUEffectBase::GetProperty(inPropertyID, inScope, inElement, outData);
}



#pragma mark -
#pragma mark audio

//--------------------------------------------------------------------------------
void SubSynth::SubSynthKernel::Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32, bool & ioSilence)
{
	const float kPIx2 = 6.283185f;

	// update internal parameters...
	float dvd = 1.0f;
	float phs = 1.0f;
	long typ = (long) GetParameter(kParam_Type);
	float filti = (typ == kMode_KeyOsc) ? 0.018f : powf(10.0f, -3.0f + (2.0f * GetParameter(kParam_Tune)));
	float filto = 1.0f - filti;
	float wet = GetParameter_scalar(kParam_Level);
	float dry = GetParameter_scalar(kParam_DryMix);
	float thr = powf(10.0f, GetParameter(kParam_Threshold) / 20.0f);
	float rls = 1.0f - powf(10.0f, -2.0f - (3.0f * GetParameter(kParam_Release)));
	float dphi = 0.0726f * kPIx2 * powf(10.0f, -2.5f + (1.5f * GetParameter(kParam_Tune)));



	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		float a = inSourceP[samp];	// process from here...

		// XXX originally this summed the stereo input channels, so here I double each channel to achieve similarly-gained results
		filt1 = (filto * filt1) + (filti * (a * 2.0f));
		filt2 = (filto * filt2) + (filti * filt1);

		float sub = filt2;
		if (sub > thr)
		{
			sub = 1.0f;
		}
		else
		{
			if (sub < -thr)
			{
				sub = -1.0f;
			}
			else
			{
				sub = 0.0f;
			}
		}

		if ((sub * dvd) < 0.0f)	// octave divider
		{
			dvd = -dvd; if (dvd < 0.0f) phs = -phs;
		}

		if (typ == kMode_Divide)	// divide
		{
			sub = phs * sub;
		}
		if (typ == kMode_Invert)	// invert
		{
			sub = phs * filt2 * 2.0f;
		}
		if (typ == kMode_KeyOsc)	// osc
		{
			if (filt2 > thr) { env = 1.0f; }
			else { env = env * rls; }
			sub = env * sinf(phi);
			phi = fmodf( phi + dphi, kPIx2 );
		}

		filt3 = (filto * filt3) + (filti * sub);
		filt4 = (filto * filt4) + (filti * filt3);

		inDestP[samp] = (a * dry) + (filt4 * wet);	// output
	}
}
