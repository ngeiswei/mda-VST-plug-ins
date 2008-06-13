#include "AUEffectBase.h"

#include "mda-common.h"


//--------------------------------------------------------------------------------
enum {
	kParam_Mode = 0,
	kParam_Dynamics,
	kParam_Mix,
	kParam_Tracking,
	kParam_Transpose,
	kParam_Maximum,
	kParam_Trigger,
	kParam_Output,
	kParam_ChannelLink,
	kNumParams
};

enum {
	kMode_Sine = 0,
	kMode_Square,
	kMode_Saw,
	kMode_Ring,
	kMode_EQ,
	kNumModes
};


//--------------------------------------------------------------------------------
class Tracker : public AUEffectBase
{
public:
	Tracker(AudioUnit inComponentInstance);
	virtual ComponentResult Initialize();
	virtual void Cleanup();
	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual ComponentResult GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
	virtual bool SupportsTail()
		{	return true;	}
	virtual ComponentResult Version()
		{	return PLUGIN_VERSION;	}
	virtual AUKernelBase * NewKernel()
		{	return new TrackerKernel(this);	}
	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);

	float GetSummedInput(UInt32 inSampleOffset)
	{
		if (summedInputBuffer != NULL)
			return summedInputBuffer[inSampleOffset];
		else
			return 0.0f;
	}

private:
	class TrackerKernel : public AUKernelBase
	{
	public:
		TrackerKernel(AUEffectBase * inAudioUnit);
		virtual void Reset();
		virtual void Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32 inNumChannels, bool & ioSilence);

	private:
		float filterFreq(float hz);

		float phi, dphi;
		float buf1, buf2, dn, bold;
		float env, saw, dsaw;
		float res1, res2, buf3, buf4;
		long min, num, sig;
	};

	float * summedInputBuffer;
};



#pragma mark -

//--------------------------------------------------------------------------------
COMPONENT_ENTRY(Tracker)

//--------------------------------------------------------------------------------
Tracker::Tracker(AudioUnit inComponentInstance)
	: AUEffectBase(inComponentInstance)
{
	summedInputBuffer = NULL;

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
Tracker::TrackerKernel::TrackerKernel(AUEffectBase * inAudioUnit)
	: AUKernelBase(inAudioUnit)
{
	res1 = cosf(0.01f); //p
	res2 = sinf(0.01f); //q
	Reset();
}

//--------------------------------------------------------------------------------
void Tracker::TrackerKernel::Reset()
{
	dphi = 100.f / (float)GetSampleRate(); //initial pitch
	min = (long) (GetSampleRate() / 30.0); //lower limit

	phi = 0.0f;
	buf1 = 0.0f;
	buf2 = 0.0f;
	dn = 0.0f;
	bold = 0.0f;
	env = 0.0f;
	saw = 0.0f;
	dsaw = 0.0f;
	buf3 = 0.0f;
	buf4 = 0.0f;
	num = 0;
	sig = 0;
}

//--------------------------------------------------------------------------------
ComponentResult	Tracker::Initialize()
{
	ComponentResult result = AUEffectBase::Initialize();

	if (result == noErr)
	{
		summedInputBuffer = (float*) malloc(GetMaxFramesPerSlice() * sizeof(float));
		if (summedInputBuffer != NULL)
		{
			for (UInt32 i=0; i < GetMaxFramesPerSlice(); i++)
				summedInputBuffer[i] = 0.0f;
		}
		else
			result = memFullErr;
	}

	return result;
}

//--------------------------------------------------------------------------------
void Tracker::Cleanup()
{
	if (summedInputBuffer != NULL)
		free(summedInputBuffer);
	summedInputBuffer = NULL;
}

//--------------------------------------------------------------------------------
ComponentResult Tracker::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
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
			outParameterInfo.defaultValue = kMode_Sine;
			break;

		case kParam_Dynamics:
			FillInParameterName(outParameterInfo, CFSTR("dynamics"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 100.0f;
			break;

		case kParam_Mix:
			FillInParameterName(outParameterInfo, CFSTR("mix"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 100.0f;
			break;

		case kParam_Tracking:
			FillInParameterName(outParameterInfo, CFSTR("glide"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 97.0f;
			break;

		case kParam_Transpose:
			FillInParameterName(outParameterInfo, CFSTR("transpose"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_RelativeSemiTones;
			outParameterInfo.minValue = -36.0f;
			outParameterInfo.maxValue = 36.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_Maximum:
			FillInParameterName(outParameterInfo, CFSTR("maximum"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = 40.0f;
			outParameterInfo.maxValue = 6309.0f;
			outParameterInfo.defaultValue = 2290.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayLogarithmic;
			break;

		case kParam_Trigger:
			FillInParameterName(outParameterInfo, CFSTR("trigger"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -60.0f;
			outParameterInfo.maxValue = 0.0f;
			outParameterInfo.defaultValue = -30.0f;
			break;

		case kParam_Output:
			FillInParameterName(outParameterInfo, CFSTR("output"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -20.0f;
			outParameterInfo.maxValue = 20.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_ChannelLink:
			FillInParameterName(outParameterInfo, CFSTR("channel link"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 1.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		default:
			result = kAudioUnitErr_InvalidParameter;
			break;
	}

	return result;
}

//--------------------------------------------------------------------------------
ComponentResult	Tracker::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_Mode:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("sine|square|saw|ring|EQ"), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}

//--------------------------------------------------------------------------------
float Tracker::TrackerKernel::filterFreq(float hz)
{
	float j, k, r=0.999f;

	j = r * r - 1;
	k = 2.f - 2.f * r * r * cosf(0.647f * hz / (float)GetSampleRate() );
	return (sqrtf(k*k - 4.f*j*j) - k) / (2.f*j);
}

//--------------------------------------------------------------------------------
OSStatus Tracker::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
	UInt32 numChannels = inBuffer.mNumberBuffers;
	float channelDivisor = 1.0f / (float)numChannels;
	channelDivisor = sqrtf(channelDivisor);	// equal-power summing
	for(UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		summedInputBuffer[samp] = 0.0f;
		for(UInt32 ch=0; ch < numChannels; ch++)
			summedInputBuffer[samp] += ((float*)(inBuffer.mBuffers[ch].mData))[samp];
		summedInputBuffer[samp] *= channelDivisor;
	}

	return AUEffectBase::ProcessBufferLists(ioActionFlags, inBuffer, outBuffer, inFramesToProcess);
}

//--------------------------------------------------------------------------------
void Tracker::TrackerKernel::Process(const Float32 * inSourceP, Float32 * inDestP, UInt32 inFramesToProcess, UInt32, bool & ioSilence)
{
	// parameter-change calcs here
	long mode = (long) GetParameter(kParam_Mode);
	float fo = filterFreq(50.f);
	float fi = (1.f - fo)*(1.f - fo);
	float trackingValue = GetParameter(kParam_Tracking) * 0.01f;
	float ddphi = trackingValue * trackingValue;
	float thr = powf(10.0f, (GetParameter(kParam_Trigger)/20.0f) - 0.8f);
	long max = (long)(GetSampleRate() / GetParameter(kParam_Maximum));
	float trans = powf(1.0594631f, GetParameter(kParam_Transpose));	// XXX Sophia changed to be continuous and not stepped
	float wet = powf(10.0f, GetParameter(kParam_Output)/20.0f);
	float dynamicsValue = GetParameter(kParam_Dynamics) * 0.01f;
	float mixValue = GetParameter(kParam_Mix) * 0.01f;
	float dry = 0.0f;
	float dyn = 0.0f;
	if(mode<kMode_EQ)
	{
		dyn = wet * 0.6f * mixValue * dynamicsValue;
		dry = wet * sqrtf(1.f - mixValue);
		wet = wet * 0.3f * mixValue * (1.f - dynamicsValue);
	}
	else
	{
		dry = wet * (1.f - mixValue);
		wet *= (0.02f*mixValue - 0.004f);
		dyn = 0.f;
	}
	float rel = (float)pow(10.0,-10.0/GetSampleRate());
	bool channelLink = (GetParameter(kParam_ChannelLink) == 0.0f) ? false : true;


	float x, t=thr, p=phi, dp=dphi, ddp=ddphi, tmp, tmp2;
	float o=fo, i=fi, b1=buf1, b2=buf2, twopi=6.2831853f;
	float we=wet, dr=dry, bo=bold, r1=res1, r2=res2, b3=buf3, b4=buf4;
	float sw=saw, dsw=dsaw, dy=dyn, e=env, re=rel;
	long	m=max, n=num, s=sig, mn=min, mo=mode;

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		if (channelLink)
			x = ((Tracker*)mAudioUnit)->GetSummedInput(samp);
		else
			x = inSourceP[samp];

		tmp = (x>0.f)? x : -x; //dynamics envelope
		e = (tmp>e)? 0.5f*(tmp + e) : e * re;

		b1 = o*b1 + i*x; 
		b2 = o*b2 + b1; //low-pass filter
		
		if(b2>t) //if >thresh
		{	
			if(s<1) //and was <thresh
			{
				if(n<mn) //not long ago
				{
					tmp2 = b2 / (b2 - bo); //update period
					tmp = trans*twopi/(n + dn - tmp2); 
					dp = dp + ddp * (tmp - dp); 
					dn = tmp2;
					dsw = 0.3183098f * dp;
					if(mode==kMode_EQ) 
					{
						r1 = cosf(4.f*dp); //resonator
						r2 = sinf(4.f*dp);
					}	
				}
				n=0; //restart period measurement
			}
			s=1;
		}
		else 
		{
			if(n>m) s=0; //now <thresh 
		}
		n++;
		bo=b2;

		p = fmodf(p+dp,twopi);
		switch(mo)
		{
			case kMode_Sine: x=sinf(p); break; 
			case kMode_Square: x=(sin(p)>0.f)? 0.5f : -0.5f; break; 
			case kMode_Saw: sw = fmodf(sw+dsw,2.0f); x = sw - 1.f; break; 
			case kMode_Ring: x *= sinf(p); break; 
			case kMode_EQ: x += (b3 * r1) - (b4 * r2); 
							b4 = 0.996f * ((b3 * r2) + (b4 * r1)); 
							b3 = 0.996f * x; break; 
		}		
		x *= (we + dy * e); 
		inDestP[samp] = dr*inSourceP[samp] + x;
	}
	if(fabsf(b1)<1.0e-10f) {buf1=0.f; buf2=0.f; buf3=0.f; buf4=0.f; } 
	else {buf1=b1; buf2=b2; buf3=b3; buf4=b4;}
	phi=p; dphi=dp; sig=s; bold=bo;
	num=(n>100000)? 100000: n; 
	env=e; saw=sw; dsaw=dsw; res1=r1; res2=r2;
}
