// SubSynth-in: "SubBass" v1.0 (based on earlier freeware "SubSynth")
// Copyright(c)1999-2006 Paul Kellett (maxim digital audio)

// 20OCT: added smoothing to FX Level

// 23OCT: denormal problems fixed

#include "AUEffectBase.h"

#include "mda-common.h"

#include "dfx-au-utilities.h"
#include "CAAUParameter.h"
#include <AudioToolbox/AudioUnitUtilities.h>


#define EXPERIMENTAL_DSP


//--------------------------------------------------------------------------------
enum
{
/*
kParam_ToneX		// Tone X	freq  |  freq  | decay
kParam_ToneY		// Tone Y	reso  |  reso  | sweep
kParam_Threshold	// Threshold	clip  | thresh | trig
*/
	kParam_Mode,
	kParam_Tune,
	kParam_Drive,
	kParam_ToneX,
	kParam_ToneY,
	kParam_Threshold,
	kParam_Limiter,
	kParam_Dry,
	kParam_Wet,
	kNumParams
};

enum
{
	kMode_Boost,
	kMode_Divide,
	kMode_Trigger,
	kNumModes
};

const long kNumPresets = 12;



//--------------------------------------------------------------------------------
class SubSynth : public AUEffectBase
{
public:
	SubSynth(AudioUnit inComponentInstance);
	virtual ~SubSynth();

	virtual ComponentResult Reset(AudioUnitScope inScope, AudioUnitElement inElement);

	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual ComponentResult GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);

	virtual ComponentResult GetPresets(CFArrayRef * outData) const;
	virtual OSStatus NewFactoryPresetSet(const AUPreset & inNewFactoryPreset);

	virtual UInt32 SupportedNumChannels(const AUChannelInfo ** outChannelInfo);
	virtual bool SupportsTail()
		{	return true;	}
	virtual ComponentResult Version()
		{	return PLUGIN_VERSION;	}

	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);


private:
	void fillpatch(long inIndex, CFStringRef inName, long inMode, float inTune, float inDrive, float inToneX, float inToneY, float inThreshold, bool inLimiter, float inDry, float inWet);
	Float32 GetParameter_scalar(AudioUnitParameterID inParameterID);

	class SubSynthPreset
	{
	public:
		SubSynthPreset();
		~SubSynthPreset();
		CFStringRef name;
		float * param;
	};
	SubSynthPreset * presets;

	// global internal variables
	long oldmode, bufpos;

	float svf0, svf1, svf2, svf3;
	float ENV, ENV2, WET, oct1, oct2;
};



#pragma mark -

//--------------------------------------------------------------------------------
COMPONENT_ENTRY(SubSynth)

//--------------------------------------------------------------------------------
SubSynth::SubSynth(AudioUnit inComponentInstance)
	: AUEffectBase(inComponentInstance)
{
	presets = new SubSynthPreset[kNumPresets];

	long i = 0;
	fillpatch(i++, CFSTR("Floppy Bass"),	kMode_Boost,   100.0f, 0.000f, 50.00f, 50.00f, -40.00f, true,  1.000f, 0.500f);
	fillpatch(i++, CFSTR("Sub Octave"),		kMode_Divide,  150.0f, 0.000f, 21.70f, 00.00f, -57.36f, true,  1.000f, 0.520f);
	fillpatch(i++, CFSTR("Reinforcements"),	kMode_Boost,   63.00f, 75.00f, 53.30f, 50.00f, -21.60f, true,  1.000f, 0.520f);
	fillpatch(i++, CFSTR("Trigger909"),		kMode_Trigger, 100.0f, 31.60f, 49.30f, 50.00f, -16.00f, false, 0.000f, 0.520f);
	fillpatch(i++, CFSTR("Square Pusher"),	kMode_Divide,  156.0f, 100.0f, 50.00f, 0.000f, -40.00f, true,  0.000f, 0.467f);
	fillpatch(i++, CFSTR("Tight EQ"),		kMode_Boost,   181.0f, 50.00f, 40.10f, 95.40f,  0.000f, true,  1.000f, 0.500f);
	fillpatch(i++, CFSTR("Added Boom"),		kMode_Trigger, 65.00f, 0.000f, 57.90f, 0.000f, -22.64f, false, 1.000f, 0.500f);
	fillpatch(i++, CFSTR("Sub Grumbler"),	kMode_Divide,  83.00f, 30.90f, 37.50f, 78.90f, -50.56f, true,  1.000f, 0.474f);
	fillpatch(i++, CFSTR("Pitch Tracker"),	kMode_Divide,  153.0f, 100.0f, 40.10f, 0.000f, -60.00f, false, 0.000f, 0.553f);
	fillpatch(i++, CFSTR("Comprezzor"),		kMode_Divide,  288.0f, 100.0f, 26.30f, 94.70f, -22.64f, true,  1.000f, 0.579f);
	fillpatch(i++, CFSTR("Hollow"),			kMode_Divide,  441.0f, 0.000f, 88.80f, 86.80f, -80.00f, false, 0.625f, 0.434f);
	fillpatch(i++, CFSTR("Space Invaders"),	kMode_Trigger, 500.0f, 59.90f, 84.90f, 100.0f, -27.36f, true,  1.000f, 0.401f);

	// init internal parameters...
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AudioUnitParameterInfo paramInfo;
		ComponentResult result = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (result == noErr)
			AUEffectBase::SetParameter(i, paramInfo.defaultValue);
	}

	Reset(kAudioUnitScope_Global, (AudioUnitElement)0);
}

//--------------------------------------------------------------------------------
SubSynth::~SubSynth()
{
	if (presets != NULL)
		delete[] presets;
	presets = NULL;
}

//--------------------------------------------------------------------------------
ComponentResult SubSynth::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	bufpos = 0;
	oldmode = -1;

	svf0 = svf1 = svf2 = svf3 = 0.0f;
	ENV = ENV2 = 0.0f;
	WET = 0.0f;
	oct1 = oct2 = 0.7f;

	return noErr;
}

//--------------------------------------------------------------------------------
UInt32 SubSynth::SupportedNumChannels(const AUChannelInfo ** outChannelInfo)
{
	static AUChannelInfo plugChannelInfo[] = { {2, 2} };

	if (outChannelInfo != NULL)
		*outChannelInfo = plugChannelInfo;

	return sizeof(plugChannelInfo) / sizeof(plugChannelInfo[0]);
}



#pragma mark -
#pragma mark parameters

//--------------------------------------------------------------------------------
ComponentResult SubSynth::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	ComponentResult result = noErr;

	outParameterInfo.defaultValue = 0.0f;
	if (presets != NULL)
		outParameterInfo.defaultValue = presets[0].param[inParameterID];
	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_Mode:
			FillInParameterName(outParameterInfo, CFSTR("Mode"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = kNumModes - 1;
			break;

		case kParam_Tune:
			FillInParameterName(outParameterInfo, CFSTR("Tune"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = 20.0f;
			outParameterInfo.maxValue = 500.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayLogarithmic;
			break;

		case kParam_Drive:
			FillInParameterName(outParameterInfo, CFSTR("Drive"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			break;

		case kParam_ToneX:
			FillInParameterName(outParameterInfo, CFSTR("Tone X"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			break;

		case kParam_ToneY:
			FillInParameterName(outParameterInfo, CFSTR("Tone Y"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			break;

		case kParam_Threshold:
			FillInParameterName(outParameterInfo, CFSTR("Threshold"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -80.0f;
			outParameterInfo.maxValue = 0.0f;
			break;

		case kParam_Limiter:
			FillInParameterName(outParameterInfo, CFSTR("Limiter"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 1.0f;
			break;

		case kParam_Dry:
			FillInParameterName(outParameterInfo, CFSTR("Dry Out"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 1.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayCubeRoot;
			break;

		case kParam_Wet:
			FillInParameterName(outParameterInfo, CFSTR("Wet Out"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 1.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayCubeRoot;
			break;

		default:
			result = kAudioUnitErr_InvalidParameter;
			break;
	}

	return result;
}

//--------------------------------------------------------------------------------
ComponentResult	SubSynth::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_Mode:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("boost|divide|trigger"), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}

//--------------------------------------------------------------------------------
Float32 SubSynth::GetParameter_scalar(AudioUnitParameterID inParameterID)
{
	return (GetParameter(inParameterID) * 0.01f);
}

#pragma mark -
#pragma mark presets

//--------------------------------------------------------------------------------
SubSynth::SubSynthPreset::SubSynthPreset()
{
	name = CFSTR("Sub-Bass Synthesizer");
	param = (float*) malloc(kNumParams * sizeof(float));
}

//--------------------------------------------------------------------------------
SubSynth::SubSynthPreset::~SubSynthPreset()
{
	if (param != NULL)
		free(param);
	param = NULL;
}

//--------------------------------------------------------------------------------
void SubSynth::fillpatch(long inIndex, CFStringRef inName, long inMode, float inTune, float inDrive, float inToneX, float inToneY, float inThreshold, bool inLimiter, float inDry, float inWet)
{
	if (presets == NULL)
		return;

	presets[inIndex].name = inName;
	presets[inIndex].param[kParam_Mode] = inMode;
	presets[inIndex].param[kParam_Tune] = inTune;
	presets[inIndex].param[kParam_Drive] = inDrive;
	presets[inIndex].param[kParam_ToneX] = inToneX;
	presets[inIndex].param[kParam_ToneY] = inToneY;
	presets[inIndex].param[kParam_Threshold] = inThreshold;
	presets[inIndex].param[kParam_Limiter] = inLimiter ? 1.0f : 0.0f;
	presets[inIndex].param[kParam_Dry] = inDry;
	presets[inIndex].param[kParam_Wet] = inWet;
}

//--------------------------------------------------------------------------
ComponentResult SubSynth::GetPresets(CFArrayRef * outData) const
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
OSStatus SubSynth::NewFactoryPresetSet(const AUPreset & inNewFactoryPreset)
{
	SInt32 presetIndex = inNewFactoryPreset.presetNumber;
	// reject an invalid preset number
	if ( (presetIndex < 0) || (presetIndex >= kNumPresets) )
		return kAudioUnitErr_InvalidPropertyValue;

	// set the parameters to the preset's values
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AUBase::SetParameter(i, kAudioUnitScope_Global, (AudioUnitElement)0, presets[presetIndex].param[i], 0);
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
OSStatus SubSynth::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
	// update internal parameters...
	float fs = GetSampleRate();
	float ifs = 1.0f / fs;

	long mode = (long) GetParameter(kParam_Mode);
	if (mode >= kNumModes)
		mode = kNumModes - 1;
	else if (mode < 0)
		mode = 0;

	long mask = (long)(0.1f * fs);	// retrig masking time

	float frq = GetParameter(kParam_Tune);
	float svff = 6.2831853f * frq * ifs;

	float drv = GetParameter_scalar(kParam_Drive);

	float dec = 1.0f - expf(-6.0f - 5.0f * GetParameter_scalar(kParam_ToneX));	// Fs dependent?

	float tonex = GetParameter_scalar(kParam_ToneX) * GetParameter_scalar(kParam_ToneX);
	if (mode == kMode_Boost)
	{
		CAAUParameter aup(GetComponentInstance(), kParam_Threshold, kAudioUnitScope_Global, 0);
		tonex = AUParameterValueToLinear(GetParameter(kParam_Threshold), &aup);
	}

	float toney = GetParameter_scalar(kParam_ToneY) * GetParameter_scalar(kParam_ToneY);
	float svfq = 1.0f - 0.95f * toney;

	float svff2;
	switch (mode)
	{
		// boom key filter
		case kMode_Trigger:
			svff2 = 5000.0f * ifs;
			break;
		default:
			svff2 = svff * expf(4.0f * GetParameter_scalar(kParam_ToneX) - 2.0f);
			break;
	}

	float thr = 2.0f * powf(10.0f, GetParameter(kParam_Threshold) / 20.0f);

	bool LIM = GetParameter(kParam_Limiter) != 0.0f;

	float dry = GetParameter(kParam_Dry);
	float wet = 4.0f * GetParameter(kParam_Wet);



	const float * in1 = (float*)(inBuffer.mBuffers[0].mData);
	const float * in2 = (float*)(inBuffer.mBuffers[1].mData);
	float * out1 = (float*)(outBuffer.mBuffers[0].mData);
	float * out2 = (float*)(outBuffer.mBuffers[1].mData);

	float env = ENV, env2 = ENV2;
	const float lt = 0.7183f;
	float s0 = svf0, s1 = svf1, s2 = svf2, s3 = svf3;

	float dw = 0.0005f * (wet - WET);
	// 20OCT wet + smooth
	if (fabsf(dw) < 1.0e-10f)
		dw = 0.0f;

	if (mode == kMode_Boost)
		drv *= 4.0f * drv;
#ifdef EXPERIMENTAL_DSP
	if (mode == kMode_Divide)
		thr *= thr;
#endif
	if (mode == kMode_Trigger)
		drv *= drv;

	if (mode != oldmode)	// reset shared buffers when changing mode
	{
		s0 = s1 = s2 = s3 = 0.0f;
		oct1 = oct2 = 0.7f;
		env = env2 = 0.0f;
		bufpos = 0;
		oldmode = mode;
	}

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		float o;

		float a = in1[samp];
		float b = in2[samp];

#ifdef EXPERIMENTAL_DSP
		float i = a + b;	// sum to mono	///anti-denormal
#else
		float i = 32768.0f * (a + b);	// sum to mono
#endif

		if (mode == kMode_Divide)	// divide
		{
			s0 -= svff * (s1 + s0 + i);	// lowpass
			s1 += svff * s0;

			i = s1 * oct1;
			// octave divider
			if (i < 0.0f)
			{
				oct1 = -oct1;
				if (oct1 < 0.0f)
					oct2 = -oct2;
			}

#ifdef EXPERIMENTAL_DSP
			if ( (i*i) < thr )
#else
			if (i < thr)
#endif
				o = 0.0f;
			else
			{
				if (drv < 0.5f)	// mix
				{
					o = 6.0f * oct2 * s1;
					o += (drv + drv) * (oct2 - o);
				}
				else
				{
					o = oct2 + (drv + drv - 1.0f) * (oct1 - oct2);
				}
			}
			s2 -= svff2 * (s3 + svfq * s2 + o); //lowpass
			o = (s3 += svff2 * s2);
		}
		else
		{
			if (mode == kMode_Trigger)	// boom
			{
				s2 += svff2 * (i - s2);	// lowpass

				i = s2;
				*(unsigned int *)&i &= 0x7FFFFFFF;
				// trigger
				if ( (i < thr) || (--bufpos > 0) )
					env2 *= dec;
				else
				{
					bufpos = mask;
					env2 = s0 = 1.0f;
					s1 = 0.0f;
				}
				i = svff + toney * (0.1f * env2 * env2 - svff);	// pitch sweep

				s0 -= i * s1;	// sine osc
				s1 += i * s0;

				s3 += 0.2f * (env2 * (s1 - drv * s1 * s1 * s1) - s3);	// 909ish tone
				o = s3;
			}
			else	// boost
			{
				// clip
				if (i > tonex)
					i = tonex;
				else if (i < -tonex)
					i = -tonex;
				s0 += svff * (i - s0);	// low-mid-dip EQ
				s1 += svff * (s0 - s1);

				s2 -= svff2 * (s3 + s2 + i - toney * i * i * i);	// rectify & bandpass
				s3 += svff2 * s2;

				o = s1 + s1 - s0 + drv * s2;
			}
		}
#ifdef EXPERIMENTAL_DSP
		o *= WET;
#else
		i = a = (dry * a) + (WET * o);	// output
		o = b = (dry * b) + (WET * o);
#endif
		WET += dw;
#ifdef EXPERIMENTAL_DSP
		i = a = (dry * a) + o;	// output
		o = b = (dry * b) + o;
#endif

		*(unsigned int *)&i &= 0x7FFFFFFF;
		*(unsigned int *)&o &= 0x7FFFFFFF;
		// detector
		if (i > o)
			o = i;
		if(o > env)
			env = o;
		else
			env *= 0.9998f;

		if (LIM)	// limiter
		{
			if (env > lt)//0.7886093f)
			{
				if (env > 1.2f)
					o = 1.0f / env;
				else
				{
					o = env - lt;
					o = 1.0f - lt * o * o;	// soft knee
				}
				a *= o;
				b *= o;
			}
		}

		out1[samp] = a;
		out2[samp] = b;
	}

#ifdef EXPERIMENTAL_DSP
	if (env2 < 0.001f)
		env2 = 0.0f;
	ENV2 = env2;	// anti-denormal
	if (env < 0.001f)
		env = 0.0f;
	ENV = env;

	if ( (s0 * s0) < 0.00001f )
		s0 = 0.0f;
	svf0 = s0;
	if ( (s1 * s1) < 0.00001f )
		s1 = 0.0f;
	svf1 = s1;
	if ( (s2 * s2) < 0.00001f )
		s2 = 0.0f;
	svf2 = s2;
	if ( (s3 * s3) < 0.00001f )
		s3 = 0.0f;
	svf3 = s3;
#else
	// anti-denormal
	if (env2 > 1.0e-8)
		ENV2 = env2;
	else
		ENV2 = 0.0f;
	if (env < 1.0e-8)
	{
		ENV = svf0 = svf1 = svf2 = svf3 = 0.0f;
	}
	else
	{
		ENV = env;
		svf0 = s0;
		svf1 = s1;
		svf2 = s2;
		svf3 = s3;
	}
#endif

	return noErr;
}
