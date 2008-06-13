#include "AUEffectBase.h"

#include "mda-common.h"


//#define RATE_PARAM_USE_HERTZ
//--------------------------------------------------------------------------------
enum {
	kParam_Mode = 0,
	kParam_Rate,
	kParam_Output,
	kNumParams
};

enum {
	kMode_Tones = 0,
	kMode_RingMod,
	kMode_TonesPlusInput,
	kNumModes
};

const long kBufferSize = 512;
const long kBufferSize_Minus1 = kBufferSize - 1;


//--------------------------------------------------------------------------------
class Shepard : public AUEffectBase
{
public:
	Shepard(AudioUnit inComponentInstance);
	virtual ~Shepard();

	virtual ComponentResult Initialize();
	virtual ComponentResult Reset(AudioUnitScope inScope, AudioUnitElement inElement);
	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual ComponentResult GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
	virtual bool SupportsTail()
		{	return true;	}
	virtual ComponentResult Version()
		{	return PLUGIN_VERSION;	}
	OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);

private:
	float pos, rate;

	float * buf1, * buf2;
};



//--------------------------------------------------------------------------------
COMPONENT_ENTRY(Shepard)

//--------------------------------------------------------------------------------
Shepard::Shepard(AudioUnit inComponentInstance)
	: AUEffectBase(inComponentInstance)
{
	buf1 = NULL;
	buf2 = NULL;

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
Shepard::~Shepard()
{
	if (buf1 != NULL)
		free(buf1);
	buf1 = NULL;
	if (buf2 != NULL)
		free(buf2);
	buf2 = NULL;
}

//--------------------------------------------------------------------------------
ComponentResult	Shepard::Initialize()
{
	ComponentResult result = AUEffectBase::Initialize();

	if (result == noErr)
	{
		// we only need to create and fill the wavetable the first time we Initialize
		if ( (buf1 == NULL) || (buf2 == NULL) )
		{
			buf1 = (float*) malloc(kBufferSize * sizeof(float));
			buf2 = (float*) malloc(kBufferSize * sizeof(float));

			const double twopi = 6.2831853;
			const double oneDivBufferRange = 1.0 / (double)(kBufferSize-1);
			for (long i=0; i < kBufferSize_Minus1; i++) 	
			{
				double tempPos = twopi * (double)i * oneDivBufferRange;	// generate wavetables
				double x = 0.0;
				double a = 1.0;
				buf2[i] = (float)sin(tempPos);
				for(int j=0; j<8; j++)
				{
					x += a * sin(fmod(tempPos,twopi));
					a *= 0.5;
					tempPos *= 2.0;
				}
				buf1[i] = (float)x;
			}
			buf1[kBufferSize-1] = 0.0f;
			buf2[kBufferSize-1] = 0.0f;	// wrap end for interpolation
		}

		Reset(kAudioUnitScope_Global, (AudioUnitElement)0);
	}

	return result;
}

//--------------------------------------------------------------------------------
ComponentResult Shepard::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	ComponentResult result = AUEffectBase::Reset(inScope, inElement);

	pos = 0.0f; 
	rate = 1.0f; 

	return result;
}

//--------------------------------------------------------------------------------
ComponentResult Shepard::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	ComponentResult result = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_Mode:
			FillInParameterName(outParameterInfo, CFSTR("mode"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0;
			outParameterInfo.maxValue = kNumModes - 1;
			outParameterInfo.defaultValue = kMode_Tones;
			break;

		case kParam_Rate:
			FillInParameterName(outParameterInfo, CFSTR("rate"), false);
#ifdef RATE_PARAM_USE_HERTZ
			outParameterInfo.unit = kAudioUnitParameterUnit_Rate;//kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = -1.25f;
			outParameterInfo.maxValue = 1.25f;
			outParameterInfo.defaultValue = 0.5f;
//			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayCubeRoot;
#else
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = -100.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 40.0f;
#endif
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
ComponentResult	Shepard::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_Mode:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("tones|ring mod|tones + in"), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}

//--------------------------------------------------------------------------------
OSStatus Shepard::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, 
	const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
	// parameter-change calcs here
	long mode = (long) GetParameter(kParam_Mode);
#ifdef RATE_PARAM_USE_HERTZ
	float drate = 1.0f + ( GetParameter(kParam_Rate) / GetSampleRate() );
#else
	float drate = 1.0f + 10.0f * powf(GetParameter(kParam_Rate)/200.0f,3.0f) / GetSampleRate();
#endif
	float out = 0.4842f * powf(10.0f, GetParameter(kParam_Output) / 20.0f);



	float r = rate, p = pos;

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		r *= drate; 
		if (r > 2.0f)
		{ 
			r *= 0.5f; 
			p *= 0.5f;
		}
		else if (r < 1.0f) 
		{
			r *= 2.f;
			p *= 2.f;
			if (p >= (float)kBufferSize_Minus1)
				p -= (float)kBufferSize_Minus1;
		}
		
		p += r; 
		if (p >= (float)kBufferSize_Minus1)
			p -= (float)kBufferSize_Minus1;

		long i1 = (long)p;	// interpolate position
		long i2 = i1 + 1;
		float di = (float)i2 - p;

		float b = di * ( buf1[i1] + (r - 2.0f) * buf2[i1] );
		b += (1.0f - di) * ( buf1[i2] + (r - 2.0f) * buf2[i2] );
		b *= out / r;

		for (UInt32 ch=0; ch < outBuffer.mNumberBuffers; ch++)
		{
			float a = ((float*)(inBuffer.mBuffers[ch].mData))[samp];
			float * outStream = (float*)(outBuffer.mBuffers[ch].mData);
			outStream[samp] = b;
			// ring mod or add
			if (mode == kMode_TonesPlusInput)
				outStream[samp] += a;
			else if (mode == kMode_RingMod)
				outStream[samp] *= a * 2.0f;
		}
	}
	pos = p;
	rate = r;

	return noErr;
}
