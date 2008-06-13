// "mda Combo" v1.0  Copyright(c)2002 Paul Kellett (@mda-vst.com)
// Based on "SampleEffectUnit" © Copyright 2002 Apple Computer, Inc. All rights reserved.

#include "AUEffectBase.h"

#include "mda-common.h"


enum
{
	_MODEL,
	_HPF,
	_DRIVE,
	_HARD,
	_BIAS,
	_OUTPUT,
	NPARAM
};

const size_t kBufferSize = 1024;


class Combo : public AUEffectBase
{
public:
	Combo(AudioUnit inComponentInstance);
	virtual AUKernelBase * NewKernel() { return new Kernel(this); }
	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual ComponentResult GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
	virtual bool SupportsTail() { return true; }
	virtual Float64 GetTailTime() { return 0.010; }	// estimate (10 ms)
	virtual ComponentResult Version() { return PLUGIN_VERSION; }

private:
	class Kernel : public AUKernelBase
	{
	public:
		Kernel(AUEffectBase * inAudioUnit);
		virtual ~Kernel();
		virtual void Process(const Float32 *inSourceP, Float32 *inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool &ioSilence);
		virtual void Reset();
		float filterFreq(float hz, float fs);

	private:
		float f0, f1, f2, f3, f4, f5, bias;
		float *buffer;
		long bufpos;
	};
};


COMPONENT_ENTRY(Combo)

Combo::Combo(AudioUnit inComponentInstance) : AUEffectBase(inComponentInstance)
{
	// init internal parameters...
	for (AudioUnitParameterID i=0; i < NPARAM; i++)
	{
		AudioUnitParameterInfo paramInfo;
		ComponentResult result = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (result == noErr)
			SetParameter(i, paramInfo.defaultValue);
	}
}


ComponentResult Combo::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo &outParameterInfo)
{
	ComponentResult result = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch(inParameterID)
	{
	case _MODEL:
		FillInParameterName(outParameterInfo, CFSTR("Speaker"), false);
		outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
		outParameterInfo.minValue = 0;
		outParameterInfo.maxValue = 6;
		outParameterInfo.defaultValue = 6;
		break;

	 case _HPF:
		FillInParameterName(outParameterInfo, CFSTR("HighPass"), false);
		outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
		outParameterInfo.minValue = 20.0;
		outParameterInfo.maxValue = 2000.0;
		outParameterInfo.defaultValue = 100.0;
		outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayLogarithmic;
		break;

	 case _DRIVE:
		FillInParameterName(outParameterInfo, CFSTR("Drive"), false);
		outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
		outParameterInfo.minValue = 0.0;
		outParameterInfo.maxValue = 100.0;
		outParameterInfo.defaultValue = 50.0;
		break;

	 case _HARD:
		FillInParameterName(outParameterInfo, CFSTR("Density"), false);
		outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
		outParameterInfo.minValue = -1.0;
		outParameterInfo.maxValue = 1.0;
		outParameterInfo.defaultValue = 0.0;
		break;

	 case _BIAS:
		FillInParameterName(outParameterInfo, CFSTR("Bias"), false);
		outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
		outParameterInfo.minValue = -1.0;
		outParameterInfo.maxValue = 1.0;
		outParameterInfo.defaultValue = 0.0;
		break;

	 case _OUTPUT:
		FillInParameterName(outParameterInfo, CFSTR("Output"), false);
		outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
		outParameterInfo.minValue = -20.0;
		outParameterInfo.maxValue = 20.0;
		outParameterInfo.defaultValue = 0.0;
		break;

	default:
		result = kAudioUnitErr_InvalidParameter;
		break;
	}
	return result;
}


ComponentResult	Combo::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings)
{
	if(inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;

	switch(inParameterID)
	{
		case _MODEL:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("Direct|Speaker Sim|Radio|MB Close|MB Distant|4x12 Centre|4x12 Edge"), CFSTR("|"));	
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;

	}
	return kAudioUnitErr_InvalidPropertyValue;
}


Combo::Kernel::Kernel(AUEffectBase * inAudioUnit)
	: AUKernelBase(inAudioUnit)
{
	buffer = (float*) malloc(kBufferSize * sizeof(float));
	Reset();
}

Combo::Kernel::~Kernel()
{
	if (buffer != NULL)
		delete[] buffer;
	buffer = NULL;
}

void Combo::Kernel::Reset()
{
	bufpos = 0;
	if (buffer != NULL)
		memset(buffer, 0, kBufferSize * sizeof(float));
	f0 = f1 = f2 = f3 = f4 = f5 = 0.0f;
}


float Combo::Kernel::filterFreq(float hz, float fs)
{
  float j, k, r=0.999f;

  j = r * r - 1.0f;
  k = 2.0f - 2.0f * r * r * (float)cos(0.647f * hz / fs);
  return (((float)sqrt(k * k - 4.0f * j * j) - k) / (2.0f * j));
}


void Combo::Kernel::Process(const Float32 *inSourceP, Float32 *inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool &ioSilence)
{
  //if ioSilence, nothing to process. Also set ioSilence to reflect plug-in output

	long  sampleFrames = inFramesToProcess;
	const float *in = inSourceP;
	float *out = inDestP;

	float fs = GetSampleRate();
	
  //parameters
	float drv = (float)exp(0.069077553f * GetParameter(_DRIVE));
	float hard = GetParameter(_HARD);
	float bias = GetParameter(_BIAS);
	float mix1 = 1.0f;
	float mix2 = -0.7f;
	long  del1 = 20;
	long  del2 = 120;
	float trim = 1.0f;
	float lpf = fs * 0.5f;
	float hpf2 = 25.0f;

	switch(int(GetParameter(_MODEL)))
	{
    case 0: //DI
			trim = 2.0f;
			lpf  = 15000.0f;
			hpf2 = 1.0f;
			mix1 = 0.0f; mix2 = 0.0f;
			del1 = 10; del2 = 10; 
			break;

    case 1: //speaker sim
			trim = 0.53f;
			lpf = 2000.0f;
			mix1 = 0.0f; mix2 = 0.0f;
			del1 = 10; del2 = 10;
			hpf2 = 382.0f;
			break;

    case 2: //radio
			trim = 0.3f;
			lpf  = 1685.0f; 
			mix1 = -1.70f; mix2 = 0.82f; 
			del1 = int(fs / 6546.0f);
			del2 = int(fs / 4315.0f); 
			break;

    case 3: //mesa boogie 1"
			trim = 0.25f;
			lpf  = 1385.0f;
			mix1 = -0.53f; mix2 = 0.21f;
			del1 = int(fs / 7345.0f);
			del2 = int(fs / 1193.0f); 
			break;

    case 4: //mesa boogie 8"
			trim = 0.25f;
			lpf  = 1685.0f;
			mix1 = -0.85f; mix2 = 0.41f; 
			del1 = int(fs / 6546.0f);
			del2 = int(fs / 3315.0f); 
			break;

    case 5: //Marshall 4x12" celestion
			trim = 0.7f;
			lpf  = 2795.0f; 
			mix1 = -0.29f; mix2 = 0.38f;         
			del1 = int(fs / 982.0f);
			del2 = int(fs / 2402.0f);
			hpf2 = 459.0f; 
			break;

    case 6: //scooped-out metal
			trim = 0.30f;
			lpf  = 1744.0f; 
			mix1 = -0.96f; mix2 = 1.6f; 
			del1 = int(fs / 356.0f);
			del2 = int(fs / 1263.0f);
			hpf2 = 382.0f; 
			break;
  }

	fs = -14.5f / fs;
	float hpf = 1.0f - (float)exp(fs * GetParameter(_HPF));
	trim *= (float)pow(10.0f, 0.05f * GetParameter(_OUTPUT) - 1.0f);
	lpf = (float)exp(fs * lpf);
	hpf2 = 1.0f - (float)exp(fs * hpf2);

	long bp = bufpos;
	float a;

	while(sampleFrames-- > 0)
	{
		a = *in - f0;  in += inNumChannels;
		f0 += hpf * a;  //hpf
		a = drv * a + bias;  //drive

		if(a > 1.0f) a = 1.0f; else if(a < -1.0f) a = -1.0f;  //clip
			a *= 1.0f - hard * (a * a - 1.0f);  //distort
		 
		*(buffer + bp) = a;

		a += mix1 * buffer[(bp + del1) & 1023] + mix2 * buffer[(bp + del2) & 1023];  //2 combs

		f1 = lpf * f1 + trim * a; //low pass
		f2 = lpf * f2 + f1;
		f3 = lpf * f3 + f2;
		f4 = lpf * f4 + f3;
		a = f4 - f5;
		f5 += hpf2 * a; //high pass

		if(--bp < 0) bp += 1024; //buffer position

		*out = a; out += inNumChannels;
	}

	bufpos = bp;
	if(fabsf(f1) + fabsf(f2) < 1.0e-10f) { f0 = f1 = f2 = f3 = f4 = f5 = 0.0f; }
}



