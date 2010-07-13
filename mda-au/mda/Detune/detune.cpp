#include "AUEffectBase.h"

#include "mda-common.h"

#include "dfx-au-utilities.h"


//--------------------------------------------------------------------------------
enum {
	kParam_Detune = 0,
	kParam_Mix,
	kParam_Output,
	kParam_WindowSize,
	kNumParams
};

inline long GetWindowSizeSamples(long inWindowSizeExponent)
{
	return (1 << inWindowSizeExponent);
}
const long kWindowSize_Min = 8;
const long kWindowSize_Max = 12;
const size_t kMaxBufferSize = GetWindowSizeSamples(kWindowSize_Max);

const float kMix_Max = 99.0f;

const long kNumChannels = 2;	// stereo effect
const long kNumPresets = 3;	// number of presets


//--------------------------------------------------------------------------------
class Detune : public AUEffectBase
{
public:
	Detune(AudioComponentInstance inComponentInstance);
	virtual ~Detune();

	virtual OSStatus Initialize();
	virtual void Cleanup();
	virtual OSStatus Reset(AudioUnitScope inScope, AudioUnitElement inElement);

	virtual OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual OSStatus GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
	virtual OSStatus SetParameter(AudioUnitParameterID inParameterID, AudioUnitScope inScope, AudioUnitElement inElement, Float32 inValue, UInt32 inBufferOffsetInFrames);

	virtual OSStatus GetPresets(CFArrayRef * outData) const;
	virtual OSStatus NewFactoryPresetSet(const AUPreset & inNewFactoryPreset);

	virtual UInt32 SupportedNumChannels(const AUChannelInfo ** outChannelInfo);
	virtual Float64 GetLatency();
	virtual Float64 GetTailTime();
	virtual bool SupportsTail()
		{	return true;	}

	virtual OSStatus Version()
		{	return PLUGIN_VERSION;	}

	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);

private:
	class DetunePreset
	{
	public:
		DetunePreset();
		~DetunePreset();
		CFStringRef name;
		float * param;
	};
	DetunePreset * presets;

	float * buf, * win;		// buffer, window
	long buflen;			// buffer length
	float semi; 			// detune display
	long pos0;				// buffer input
	float pos1, dpos1;		// buffer output, rate
	float pos2, dpos2;		// downwards shift
};



#pragma mark -

//--------------------------------------------------------------------------------
COMPONENT_ENTRY(Detune)

//--------------------------------------------------------------------------------
Detune::Detune(AudioComponentInstance inComponentInstance)
	: AUEffectBase(inComponentInstance)
{
	// initialise...
	buf = NULL;
	win = NULL;
	buflen = 0;

	presets = new DetunePreset[kNumPresets];

	// init internal parameters...
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AudioUnitParameterInfo paramInfo;
		OSStatus status = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (status == noErr)
		{
			AUEffectBase::SetParameter(i, paramInfo.defaultValue);
			for (long presetNum=0; presetNum < kNumPresets; presetNum++)
				presets[presetNum].param[i] = paramInfo.defaultValue;
		}
	}

	// differences from default program...
	presets[1].name = CFSTR("Symphonic");
	presets[1].param[kParam_Detune] = 2.4f;
//	presets[1].param[kParam_WindowSize] = kWindowSize_Max;//0.9f;	// XXX yah?
	presets[2].name = CFSTR("Out Of Tune");
	presets[2].param[kParam_Detune] = 153.6f;
	presets[2].param[kParam_Mix] = 0.7f * kMix_Max;
}


//--------------------------------------------------------------------------------
Detune::~Detune()
{
	if (presets != NULL)
		delete [] presets;
	presets = NULL;
}


//--------------------------------------------------------------------------------
OSStatus Detune::Initialize()
{
	OSStatus status = AUEffectBase::Initialize();
	if (status == noErr)
	{
		buf = (float*) malloc(kMaxBufferSize * sizeof(float));;
		win = (float*) malloc(kMaxBufferSize * sizeof(float));;

		Reset(kAudioUnitScope_Global, (AudioUnitElement)0);
	}
	return status;
}

//--------------------------------------------------------------------------------
// destroy any buffers...
void Detune::Cleanup()
{
	if (buf != NULL)
		free(buf);
	buf = NULL;
	if (win != NULL)
		free(win);
	win = NULL;
}


//--------------------------------------------------------------------------------
// clear any buffers...
OSStatus Detune::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	if (buf != NULL)
		memset(buf, 0, kMaxBufferSize * sizeof(float));
	pos0 = 0;
	pos1 = pos2 = 0.0f;

	return noErr;
}


#pragma mark -
#pragma mark parameters

//--------------------------------------------------------------------------------
OSStatus Detune::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	OSStatus status = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_Detune:
			FillInParameterName(outParameterInfo, CFSTR("detune"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Cents;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 300.0f;
			outParameterInfo.defaultValue = 19.2f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayCubeRoot;
			break;

		case kParam_Mix:
			FillInParameterName(outParameterInfo, CFSTR("mix"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = kMix_Max;
			outParameterInfo.defaultValue = 39.6f;
			break;

		case kParam_Output:
			FillInParameterName(outParameterInfo, CFSTR("output"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -20.0f;
			outParameterInfo.maxValue = 20.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_WindowSize:
			FillInParameterName(outParameterInfo, CFSTR("latency"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = kWindowSize_Min;
			outParameterInfo.maxValue = kWindowSize_Max;
			outParameterInfo.defaultValue = (kWindowSize_Min + kWindowSize_Max) / 2;
			break;

		default:
			status = kAudioUnitErr_InvalidParameter;
			break;
	}

	return status;
}

//--------------------------------------------------------------------------
OSStatus Detune::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_WindowSize:
			if (outStrings != NULL)
			{
				// allocate a mutable array large enough to hold all of the value strings
				CFMutableArrayRef mutableArray = CFArrayCreateMutable(kCFAllocatorDefault, (kWindowSize_Max - kWindowSize_Min) + 1, &kCFTypeArrayCallBacks);
				if (mutableArray != NULL)
				{
					for (long i=kWindowSize_Min; i <= kWindowSize_Max; i++)
					{
						CFStringRef valueString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, 
														CFSTR("%ld samples"), GetWindowSizeSamples(i));
						if (valueString != NULL)
						{
							CFArrayAppendValue(mutableArray, valueString);
							CFRelease(valueString);
						}
					}
					*outStrings = (CFArrayRef)mutableArray;
				}
				else
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}

//--------------------------------------------------------------------------
OSStatus Detune::SetParameter(AudioUnitParameterID inParameterID, AudioUnitScope inScope, AudioUnitElement inElement, Float32 inValue, UInt32 inBufferOffsetInFrames)
{
	OSStatus status = AUBase::SetParameter(inParameterID, inScope, inElement, inValue, inBufferOffsetInFrames);
	if (status == noErr)
	{
		if (inParameterID == kParam_WindowSize)
		{
			PropertyChanged(kAudioUnitProperty_Latency, kAudioUnitScope_Global, (AudioUnitElement)0);
			PropertyChanged(kAudioUnitProperty_TailTime, kAudioUnitScope_Global, (AudioUnitElement)0);
		}
	}
	return status;
}



#pragma mark -
#pragma mark presets

//--------------------------------------------------------------------------------
Detune::DetunePreset::DetunePreset()
{
	name = CFSTR("Stereo Detune");
	param = (float*) malloc(kNumParams * sizeof(float));
}

//--------------------------------------------------------------------------------
Detune::DetunePreset::~DetunePreset()
{
	if (param != NULL)
		free(param);
	param = NULL;
}

//--------------------------------------------------------------------------
OSStatus Detune::GetPresets(CFArrayRef * outData) const
{
	// this is just to say that the property is supported (GetPropertyInfo needs this)
	if (outData == NULL)
		return noErr;

	// allocate a mutable array large enough to hold them all
	CFMutableArrayRef outArray = CFArrayCreateMutable(kCFAllocatorDefault, kNumPresets, &kCFAUPresetArrayCallBacks);
	if (outArray == NULL)
	{
		*outData = NULL;
		return coreFoundationUnknownErr;
	}

	// add the preset data (name and number) into the array
	for (long i=0; i < kNumPresets; i++)
	{
		// set the data as it should be
		CFAUPresetRef aupreset = CFAUPresetCreate(kCFAllocatorDefault, i, presets[i].name);
		if (aupreset != NULL)
		{
			// insert the AUPreset into the output array
			CFArrayAppendValue(outArray, aupreset);
			CFAUPresetRelease(aupreset);
		}
	}

	*outData = (CFArrayRef)outArray;

	return noErr;
}

//--------------------------------------------------------------------------
OSStatus Detune::NewFactoryPresetSet(const AUPreset & inNewFactoryPreset)
{
	SInt32 presetIndex = inNewFactoryPreset.presetNumber;
	// reject an invalid preset number
	if ( (presetIndex < 0) || (presetIndex >= kNumPresets) )
		return kAudioUnitErr_InvalidPropertyValue;

	// set the parameters to the preset's values
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		SetParameter(i, kAudioUnitScope_Global, (AudioUnitElement)0, presets[presetIndex].param[i], 0);
		AUParameterChange_TellListeners(GetComponentInstance(), i);
	}

	// update what we report as the current loaded preset
	AUPreset au_preset;
	au_preset.presetNumber = inNewFactoryPreset.presetNumber;
	au_preset.presetName = presets[presetIndex].name;
	SetAFactoryPresetAsCurrent(au_preset);

	return noErr;
}



#pragma mark -
#pragma mark audio

//--------------------------------------------------------------------------------
UInt32 Detune::SupportedNumChannels(const AUChannelInfo ** outChannelInfo)
{
	static AUChannelInfo plugChannelInfo[] = { {kNumChannels, kNumChannels} };

	if (outChannelInfo != NULL)
		*outChannelInfo = plugChannelInfo;

	return sizeof(plugChannelInfo) / sizeof(plugChannelInfo[0]);
}

//--------------------------------------------------------------------------------
Float64 Detune::GetLatency()
{
	return (Float64)GetWindowSizeSamples((long)GetParameter(kParam_WindowSize)) / GetSampleRate();
}

//--------------------------------------------------------------------------------
Float64 Detune::GetTailTime()
{
	return GetLatency();
}


//--------------------------------------------------------------------------------
OSStatus Detune::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
	semi = GetParameter(kParam_Detune) * 0.01f;
	dpos2 = powf(1.0594631f, semi);
	dpos1 = 1.0f / dpos2;

	long tmp = GetWindowSizeSamples( (long)GetParameter(kParam_WindowSize) );
	if (tmp != buflen) //recalculate crossfade window
	{
		buflen = tmp;

		// hanning half-overlap-and-add
		double p = 0.0, dp = 6.28318530718 / (double)buflen;
		for (long i=0; i < buflen; i++)
		{
			win[i] = 0.5f - 0.5f * cosf(p);
			p += dp;
		}
	}

	float wet = powf(10.0f, GetParameter(kParam_Output) / 20.0f);
	float mixValue = GetParameter(kParam_Mix) / kMix_Max;
	float dry = wet - wet * mixValue * mixValue;
	wet = (wet + wet - wet * mixValue) * mixValue;



	const float * in1 = (float*) (inBuffer.mBuffers[0].mData);
	const float * in2 = (float*) (inBuffer.mBuffers[1].mData);
	float * out1 = (float*) (outBuffer.mBuffers[0].mData);
	float * out2 = (float*) (outBuffer.mBuffers[1].mData);

	float a, b, c, d;
	float x, w=wet, y=dry, p1=pos1, p1f, d1=dpos1;
	float p2=pos2, d2=dpos2;
	long p0=pos0, p1i, p2i;
	long l=buflen-1, lh=buflen>>1;
	float lf = (float)buflen;

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		a = in1[samp];
		b = in2[samp];

		c = y * a;
		d = y * b;

		--p0 &= l; 		
		buf[p0] = w * (a + b);			//input

		p1 -= d1;
		if(p1<0.0f) p1 += lf; 					//output
		p1i = (long)p1;
		p1f = p1 - (float)p1i;
		a = buf[p1i];
		++p1i &= l;
		a += p1f * (buf[p1i] - a);	//linear interpolation

		p2i = (p1i + lh) & l; 					//180-degree ouptut
		b = buf[p2i];
		++p2i &= l;
		b += p1f * (buf[p2i] - b);	//linear interpolation

		p2i = (p1i - p0) & l; 					//crossfade window
		x = *(win + p2i);
		//++p2i &= l;
		//x += p1f * (*(win + p2i) - x); //linear interpolation (doesn't do much)
		c += b + x * (a - b);

		p2 -= d2;	//repeat for downwards shift - can't see a more efficient way?
		if(p2<0.0f) p2 += lf; 					//output
		p1i = (long)p2;
		p1f = p2 - (float)p1i;
		a = buf[p1i];
		++p1i &= l;
		a += p1f * (buf[p1i] - a);	//linear interpolation

		p2i = (p1i + lh) & l; 					//180-degree ouptut
		b = buf[p2i];
		++p2i &= l;
		b += p1f * (buf[p2i] - b);	//linear interpolation

		p2i = (p1i - p0) & l; 					//crossfade window
		x = *(win + p2i);
		//++p2i &= l;
		//x += p1f * (*(win + p2i) - x); //linear interpolation (doesn't do much)
		d += b + x * (a - b);

		out1[samp] = c;
		out2[samp] = d;
	}
	pos0=p0; pos1=p1; pos2=p2;

	return noErr;
}
