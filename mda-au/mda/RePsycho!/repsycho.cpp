#include "AUEffectBase.h"

#include "mda-common.h"


//--------------------------------------------------------------------------------
enum {
	kParam_Tune = 0,
	kParam_FineTune,
	kParam_Env,
	kParam_Thresh,
	kParam_MinimumChunkLength,
	kParam_Mix,
	kParam_Quality,
	kNumParams
};

//const long kBufferSize = 22050;
const float kBufferSize_seconds = 0.5f;


//--------------------------------------------------------------------------------
class RePsycho : public AUEffectBase
{
public:
	RePsycho(AudioUnit inComponentInstance);

	virtual ComponentResult Initialize();

	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual ComponentResult GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);

	virtual bool SupportsTail()
		{	return true;	}
	virtual Float64 GetTailTime()
		{	return kBufferSize_seconds;	}	// actually less, but this is fine enough

	virtual ComponentResult Version()
		{	return PLUGIN_VERSION;	}
	virtual AUKernelBase * NewKernel()
		{	return new RePsychoKernel(this);	}

	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, 
						const AudioBufferList & inBuffer, AudioBufferList & outBuffer, 
						UInt32 inFramesToProcess);
	float GetAverageAmplitude(UInt32 inSampleFrame);

private:
	class RePsychoKernel : public AUKernelBase
	{
	public:
		RePsychoKernel(AUEffectBase * inAudioUnit);
		virtual ~RePsychoKernel();
		virtual void Reset();
		virtual void Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool & ioSilence);

		void UpdateBuffer();

	private:
		float buf;
		long allocatedBufferSize;
		float * buffer;
		long tim;
		float gai;
	};

	float numChannelsScalar;
	AudioBuffer * inputAudioBuffers;
	UInt32 numInputAudioBuffers;
};



#pragma mark -

//--------------------------------------------------------------------------------
COMPONENT_ENTRY(RePsycho)

//--------------------------------------------------------------------------------
RePsycho::RePsycho(AudioUnit inComponentInstance)
	: AUEffectBase(inComponentInstance)
{
	// init internal parameters...
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AudioUnitParameterInfo paramInfo;
		ComponentResult result = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (result == noErr)
			AUEffectBase::SetParameter(i, paramInfo.defaultValue);
	}
}

//--------------------------------------------------------------------------------
RePsycho::RePsychoKernel::RePsychoKernel(AUEffectBase * inAudioUnit)
	: AUKernelBase(inAudioUnit)
{
	buffer = NULL;
	allocatedBufferSize = 0;
	UpdateBuffer();

	Reset();
}

//--------------------------------------------------------------------------------
RePsycho::RePsychoKernel::~RePsychoKernel()
{
	if (buffer != NULL)
		free(buffer);
	buffer = NULL;
	allocatedBufferSize = 0;
}

//--------------------------------------------------------------------------------
ComponentResult	RePsycho::Initialize()
{
	ComponentResult result = AUEffectBase::Initialize();

	if (result == noErr)
	{
		for (KernelList::iterator it = mKernelList.begin(); it != mKernelList.end(); it++)
		{
			RePsychoKernel * kernel = (RePsychoKernel*)(*it);
			if (kernel != NULL)
				kernel->UpdateBuffer();
		}

		Reset(kAudioUnitScope_Global, (AudioUnitElement)0);
	}

	return result;
}

//--------------------------------------------------------------------------------
void RePsycho::RePsychoKernel::Reset()
{
	if (buffer != NULL)
		memset(buffer, 0, allocatedBufferSize * sizeof(float));

	buf = 0.0f;
	gai = 0.0f;	// XXX 1.0f ?
}

//--------------------------------------------------------------------------------
void RePsycho::RePsychoKernel::UpdateBuffer()
{
	long newSize = (long) (kBufferSize_seconds * GetSampleRate());
	if (newSize != allocatedBufferSize)
	{
		if (buffer != NULL)
			free(buffer);
		allocatedBufferSize = newSize;
		buffer = (float*) malloc(allocatedBufferSize * sizeof(float));

		tim = allocatedBufferSize + 1;
	}
}



#pragma mark -
#pragma mark parameters

//--------------------------------------------------------------------------------
ComponentResult RePsycho::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	ComponentResult result = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_Tune:
			FillInParameterName(outParameterInfo, CFSTR("tune"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_RelativeSemiTones;
			outParameterInfo.minValue = -24.0f;
			outParameterInfo.maxValue = 0.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_FineTune:
			FillInParameterName(outParameterInfo, CFSTR("fine"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Cents;
			outParameterInfo.minValue = -100.0f;
			outParameterInfo.maxValue = 0.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_Env:
			FillInParameterName(outParameterInfo, CFSTR("decay"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = -50.0f;
			outParameterInfo.maxValue = 50.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_Thresh:
			FillInParameterName(outParameterInfo, CFSTR("thresh"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -30.0f;
			outParameterInfo.maxValue = 0.0f;
			outParameterInfo.defaultValue = -12.0f;
			break;

		case kParam_MinimumChunkLength:
			FillInParameterName(outParameterInfo, CFSTR("hold"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Milliseconds;
			outParameterInfo.minValue = 10.0f;
			outParameterInfo.maxValue = (kBufferSize_seconds*1000.0f * 0.5f) + outParameterInfo.minValue;
			outParameterInfo.defaultValue = ((outParameterInfo.maxValue - outParameterInfo.minValue) * 0.45) + outParameterInfo.minValue;
			break;

		case kParam_Mix:
			FillInParameterName(outParameterInfo, CFSTR("mix"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 100.0f;
			break;

		case kParam_Quality:
			FillInParameterName(outParameterInfo, CFSTR("quality"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 1.0f;
			outParameterInfo.defaultValue = 1.0f;
			break;

		default:
			result = kAudioUnitErr_InvalidParameter;
			break;
	}

	return result;
}

//--------------------------------------------------------------------------------
ComponentResult	RePsycho::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_Quality:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("low|high"), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}



#pragma mark -
#pragma mark audio

//--------------------------------------------------------------------------------
OSStatus RePsycho::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, 
						const AudioBufferList & inBuffer, AudioBufferList & outBuffer, 
						UInt32 inFramesToProcess)
{
	numChannelsScalar = 1.0f / (float)(inBuffer.mNumberBuffers);
	inputAudioBuffers = (AudioBuffer*) (inBuffer.mBuffers);
	numInputAudioBuffers = inBuffer.mNumberBuffers;

	return AUEffectBase::ProcessBufferLists(ioActionFlags, inBuffer, outBuffer, inFramesToProcess);
}

//--------------------------------------------------------------------------------
float RePsycho::GetAverageAmplitude(UInt32 inSampleFrame)
{
	float averageAmplitude = 0.0f;
	for (UInt32 i=0; i < numInputAudioBuffers; i++)
		averageAmplitude += fabsf( ((Float32*)(inputAudioBuffers[i].mData))[inSampleFrame] );
	averageAmplitude *= numChannelsScalar;
	return averageAmplitude;
}

//--------------------------------------------------------------------------------
void RePsycho::RePsychoKernel::Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool & ioSilence)
{
	// calcs here
	long dtim = (long) (GetParameter(kParam_MinimumChunkLength) * 0.001f * GetSampleRate());
	float thr = powf(10.0f, GetParameter(kParam_Thresh) / 20.0f);
//thr *= 0.5f;	// XXX cuz we're not summing anymore when comparing?

	float env = GetParameter(kParam_Env) * 0.01f;
	if (GetParameter(kParam_Env) > 0.0f) 
		env = 1.0f + 0.003f * powf(env, 5.0f);
	else
		env = 1.0f + 0.025f * powf(env, 5.0f);

	float tun = (( (float)((long)GetParameter(kParam_Tune)) + (GetParameter(kParam_FineTune) * 0.01f) ) / 24.0f);
	tun = powf(10.0f, 0.60206f * tun);
	float mixLevel = GetParameter(kParam_Mix) * 0.01f;
	float wet = sqrtf(mixLevel);
	float dry = sqrtf(1.0f - mixLevel);
	bool highQuality = (GetParameter(kParam_Quality) >= 1.0f);



	float x = 0.0f;

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		float a = inSourceP[samp];		

		if ( (((RePsycho*)mAudioUnit)->GetAverageAmplitude(samp) > thr) && (tim > dtim) )	// trigger
		{
			gai = 1.0f;
			tim = 0;
		}

		if (tim < allocatedBufferSize)	// play out
		{	
			if (tim < 80)	// fade in
			{
				if (tim == 0)
					buf = x;

				buffer[tim] = a;
				x = buffer[(long)((float)tim * tun)];

				x = buf * (1.0f - (0.0125f * (float)tim)) + (x * 0.0125f * (float)tim);
			}
			else
			{
				// update to/from buffer
				buffer[tim] = a;

				if (highQuality)
				{
					// interpolation
					float it1 = (float)tim * tun;
					long of1 = (long)it1;
					long of2 = of1 + 1;
					it1 = it1 - (float)of1;
					float it2 = 1.0f - it1;

					x = (it2 * buffer[of1]) + (it1 * buffer[of2]);
				}
				else
				{
					x = buffer[(long)(tim * tun)];
				}
			}

			tim++;
			gai *= env;
		}
		else	// mute
		{
			gai = 0.0f;
		}

		inDestP[samp] = (a * dry) + (x * gai * wet);	// output
	}
}
