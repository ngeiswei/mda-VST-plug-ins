#include "AUEffectBase.h"

#include "mda-common.h"


#define USE_SOPHIA_PARAMETERS
//--------------------------------------------------------------------------------
enum {
	kParam_Delay = 0,
	kParam_Feedback,
#ifdef USE_SOPHIA_PARAMETERS
	kParam_FeedbackMode,
#endif
	kParam_FeedbackTone,
	kParam_LFODepth,
	kParam_LFORate,
	kParam_WetMix,
	kParam_Output,
	kNumParams
};

#ifdef USE_SOPHIA_PARAMETERS
enum {
	kFeedbackMode_Limit,
	kFeedbackMode_Saturate,
	kNumFeedbackModes
};
#endif

const float kMaxDelayTime = 16.0f;	// in seconds


//--------------------------------------------------------------------------------
class DubDelay : public AUEffectBase
{
public:
	DubDelay(AudioUnit inComponentInstance);

	virtual ComponentResult Initialize();
	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
#ifdef USE_SOPHIA_PARAMETERS
	virtual ComponentResult GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
#endif
	virtual ComponentResult Version()
		{	return PLUGIN_VERSION;	}
	virtual AUKernelBase * NewKernel()
		{	return new DubDelayKernel(this);	}

private:
	class DubDelayKernel : public AUKernelBase
	{
	public:
		DubDelayKernel(AUEffectBase * inAudioUnit);
		virtual ~DubDelayKernel();
		virtual void Reset();
		virtual void Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool & ioSilence);

		void UpdateBuffer();

	private:
		float * buffer;		// delay
		long allocatedBufferSize, ipos;	// delay max time, pointer, left time, right time
		float fil0;			// crossover filter buffer
		float env;			// limiter (clipper when release is instant)
		float phi;			// lfo
		float dlbuf;		// smoothed modulated delay
	};
};



#pragma mark -

//--------------------------------------------------------------------------------
COMPONENT_ENTRY(DubDelay)

//--------------------------------------------------------------------------------
DubDelay::DubDelay(AudioUnit inComponentInstance)
	: AUEffectBase(inComponentInstance)
{
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
DubDelay::DubDelayKernel::DubDelayKernel(AUEffectBase * inAudioUnit)
	: AUKernelBase(inAudioUnit)
{
	buffer = NULL;
	allocatedBufferSize = 0;
	UpdateBuffer();

	Reset();
}

//--------------------------------------------------------------------------------
DubDelay::DubDelayKernel::~DubDelayKernel()
{
	if (buffer != NULL)
		free(buffer);
	buffer = NULL;
	allocatedBufferSize = 0;
}

//--------------------------------------------------------------------------------
ComponentResult DubDelay::Initialize()
{
	ComponentResult result = AUEffectBase::Initialize();

	if (result == noErr)
	{
		for (KernelList::iterator it = mKernelList.begin(); it != mKernelList.end(); it++)
		{
			DubDelayKernel * kernel = (DubDelayKernel*)(*it);
			if (kernel != NULL)
				kernel->UpdateBuffer();
		}

		Reset(kAudioUnitScope_Global, (AudioUnitElement)0);
	}

	return result;
}

//--------------------------------------------------------------------------------
void DubDelay::DubDelayKernel::Reset()
{
	if (buffer != NULL)
		memset(buffer, 0, allocatedBufferSize * sizeof(float));

	ipos = 0;
	fil0 = 0.0f;
	env	= 0.0f;
	phi	= 0.0f;
	dlbuf= 0.0f;
}

//--------------------------------------------------------------------------------
void DubDelay::DubDelayKernel::UpdateBuffer()
{
	long newSize = (long) (kMaxDelayTime * GetSampleRate());
	if (newSize != allocatedBufferSize)
	{
		if (buffer != NULL)
			free(buffer);
		allocatedBufferSize = newSize;
		buffer = (float*) malloc((allocatedBufferSize + 2) * sizeof(float));	// spare just in case!
	}
}

//--------------------------------------------------------------------------------
ComponentResult DubDelay::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	ComponentResult result = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_Delay:
			FillInParameterName(outParameterInfo, CFSTR("delay"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Milliseconds;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = kMaxDelayTime * 1000.0f;
			outParameterInfo.defaultValue = 500.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplaySquareRoot;
			break;

		case kParam_Feedback:
			FillInParameterName(outParameterInfo, CFSTR("feedback"), false);
#ifdef USE_SOPHIA_PARAMETERS
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
#else
			outParameterInfo.unit = kAudioUnitParameterUnit_CustomUnit;
			outParameterInfo.unitName = CFSTR("sat <-> limit");
			outParameterInfo.minValue = -110.0f;
#endif
			outParameterInfo.maxValue = 110.0f;
			outParameterInfo.defaultValue = 44.0f;
			break;

#ifdef USE_SOPHIA_PARAMETERS
		case kParam_FeedbackMode:
			FillInParameterName(outParameterInfo, CFSTR("feedback mode"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = (float) (kNumFeedbackModes - 1);
			outParameterInfo.defaultValue = (float) kFeedbackMode_Limit;
			break;
#endif

		case kParam_FeedbackTone:
			FillInParameterName(outParameterInfo, CFSTR("feedback tone"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_CustomUnit;
			outParameterInfo.unitName = CFSTR("low <-> high");
			outParameterInfo.minValue = -100.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = -20.0f;
			break;

		case kParam_LFODepth:
			FillInParameterName(outParameterInfo, CFSTR("LFO depth"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_LFORate:
			FillInParameterName(outParameterInfo, CFSTR("LFO rate"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = 0.01f;
			outParameterInfo.maxValue = 10.0f;
			outParameterInfo.defaultValue = 3.0f;
//			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayLogarithmic;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplaySquareRoot;
			break;

		case kParam_WetMix:
			FillInParameterName(outParameterInfo, CFSTR("FX mix"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 33.0f;
			break;

		case kParam_Output:
			FillInParameterName(outParameterInfo, CFSTR("output"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 2.0f;
			outParameterInfo.defaultValue = 1.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayCubeRoot;
			break;

		default:
			result = kAudioUnitErr_InvalidParameter;
			break;
	}

	return result;
}

#ifdef USE_SOPHIA_PARAMETERS
//--------------------------------------------------------------------------------
ComponentResult DubDelay::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_FeedbackMode:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("limit|saturate"), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}
#endif

//--------------------------------------------------------------------------------
void DubDelay::DubDelayKernel::Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool & ioSilence)
{
	float fs=GetSampleRate();
	if(fs < 8000.0f) fs = 44100.0f; //??? bug somewhere!

	// calcs here
	///CHANGED///del = GetParameter(kParam_Delay) * GetParameter(kParam_Delay) * GetParameter(kParam_Delay) * (float)allocatedBufferSize;
	float del = GetParameter(kParam_Delay)*0.001f * fs;
	if (del > (float)allocatedBufferSize)
		del = (float)allocatedBufferSize;
	float mod = 0.049f * GetParameter(kParam_LFODepth)*0.01f * del;
	
	float lmix, hmix;
	float fil = GetParameter(kParam_FeedbackTone) * 0.01f;
	if(fil > 0.0f)	//simultaneously change crossover frequency & high/low mix
	{
		fil = 0.25f * fil;
		lmix = -2.0f * fil;
		hmix = 1.0f;
	}
	else 
	{ 
		hmix = fil + 1.0f;
		lmix = 1.0f - hmix;
	}
	fil = expf(-6.2831853f * powf(10.0f, 2.2f + 4.5f * fil) / fs);

#ifdef USE_SOPHIA_PARAMETERS
	long feedbackMode = (long) GetParameter(kParam_FeedbackMode);
	float fbk = GetParameter(kParam_Feedback)*0.01f;
#else
	long feedbackMode = (GetParameter(kParam_Feedback) > 0.0f) ? kFeedbackMode_Limit : kFeedbackMode_Saturate;
	float fbk = fabsf(GetParameter(kParam_Feedback)*0.01f);
#endif
	float rel;
	switch (feedbackMode)
	{
		case kFeedbackMode_Limit:
			rel = 0.8f; //limit or clip
			break;
		case kFeedbackMode_Saturate:
		default:
			rel = 0.9997f;
			break;
	}

	float wet = 1.0f - (GetParameter(kParam_WetMix)*0.01f);
	wet = GetParameter(kParam_Output)*0.5f * (1.0f - wet * wet); //-3dB at 50% mix
	float dry = GetParameter(kParam_Output) * (1.0f - (GetParameter(kParam_WetMix)*0.01f) * (GetParameter(kParam_WetMix)*0.01f));

	float dphi = 628.31853f * GetParameter(kParam_LFORate) / fs; //100-sample steps



	float ol, w=wet, y=dry, fb=fbk, dl=dlbuf, db=dlbuf, ddl = 0.0f;
	float lx=lmix, hx=hmix, f=fil, f0=fil0, tmp;
	float e=env, g, r=rel; //limiter envelope, gain, release
	float twopi=6.2831853f;
	long	i=ipos, l, s=allocatedBufferSize, k=0;

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		if(k==0) //update delay length at slower rate (could be improved!)
		{
			db += 0.01f * (del - db - mod - mod * sinf(phi)); //smoothed delay+lfo
			ddl = 0.01f * (db - dl); //linear step
			phi+=dphi; if(phi>twopi) phi-=twopi;
			k=100;
		}
		k--;
		dl += ddl; //lin interp between points
 
		i--; if(i<0) i=s; 												//delay positions
		
		l = (long)dl;
		tmp = dl - (float)l; //remainder
		l += i; if(l>s) l-=(s+1);
		
		ol = *(buffer + l); 											//delay output
		
		l++; if(l>s) l=0; 
		ol += tmp * (*(buffer + l) - ol); //lin interp

		tmp = inSourceP[samp] + fb * ol;								//mix input (left only!) & feedback

		f0 = f * (f0 - tmp) + tmp;								//low-pass filter
		tmp = lx * f0 + hx * tmp;
		
		g =(tmp<0.0f)? -tmp : tmp;								//simple limiter
		e *= r; if(g>e) e = g;
		if(e>1.0f) tmp /= e;

		*(buffer + i) = tmp;									//delay input
		
		ol *= w;												//wet

		inDestP[samp] = y * inSourceP[samp] + ol; 					//dry
	}
	ipos = i;
	dlbuf=dl;
	if(fabsf(f0)<1.0e-10f) { fil0=0.0f; env=0.0f; } else { fil0=f0; env=e; } //trap denormals
}

