#include "AUEffectBase.h"
#include "CAAUParameter.h"
#include <AudioToolbox/AudioUnitUtilities.h>

#include "mda-common.h"

#include "dfx-au-utilities.h"


//--------------------------------------------------------------------------------
enum {
	kParam_SpeedMode = 0,
	kParam_LowWidth,
	kParam_LowThrob,
	kParam_HighWidth,
	kParam_HighDepth,
	kParam_HighThrob,
	kParam_Crossover,
	kParam_Output,
	kParam_Speed,
	kNumParams
};

enum {
	kSpeedMode_Stop = 0,
	kSpeedMode_Slow,
	kSpeedMode_Fast,
	kNumSpeedModes
};

const float kTwoPi = 6.2831853f;
const size_t kBufferSize = 256;

const float kCrossoverMin = 150.0f;
const float kCrossoverMax = 1500.0f;

const long kNumInputs = 1;
const long kNumOutputs = 2;
const long kNumPresets = 3;	// number of presets


//--------------------------------------------------------------------------------
class Leslie : public AUEffectBase
{
public:
	Leslie(AudioComponentInstance inComponentInstance);
	virtual ~Leslie();

	virtual OSStatus Initialize();
	virtual void Cleanup();
	virtual OSStatus Reset(AudioUnitScope inScope, AudioUnitElement inElement);

	virtual OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual OSStatus GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);

	virtual OSStatus GetPresets(CFArrayRef * outData) const;
	virtual OSStatus NewFactoryPresetSet(const AUPreset & inNewFactoryPreset);

	virtual UInt32 SupportedNumChannels(const AUChannelInfo ** outChannelInfo);
//	virtual Float64 GetLatency();
//	virtual Float64 GetTailTime();
	virtual bool SupportsTail()
		{	return true;	}
#if 0
	virtual void SetBypassEffect(bool inFlag);
#endif

	virtual OSStatus Version()
		{	return PLUGIN_VERSION;	}

	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);


private:
	class LesliePreset
	{
	public:
		LesliePreset();
		~LesliePreset();
		CFStringRef name;
		float * param;
	};
	LesliePreset * presets;

	float fbuf1, fbuf2;	// filter buffers
	float hspd, hphi;
	float lspd, lphi;
	float * hbuf;	// HF delay buffer
	long hpos;	// buffer length & pointer
};



#pragma mark -

//--------------------------------------------------------------------------------
COMPONENT_ENTRY(Leslie)

//--------------------------------------------------------------------------------
Leslie::Leslie(AudioComponentInstance inComponentInstance)
	: AUEffectBase(inComponentInstance, false)
{
	hbuf = NULL;

	presets = new LesliePreset[kNumPresets]; 

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

	presets[1].name = CFSTR("Slow");
	presets[1].param[kParam_SpeedMode] = kSpeedMode_Slow;
	presets[1].param[kParam_HighDepth] = 75.0f;
	presets[1].param[kParam_HighThrob] = 57.0f;
	presets[2].name = CFSTR("Fast");
	presets[2].param[kParam_SpeedMode] = kSpeedMode_Fast;
	presets[2].param[kParam_HighDepth] = 60.0f;
	presets[2].param[kParam_HighThrob] = 70.0f;

/*
	CreateElements();
	CAStreamBasicDescription streamDesc = GetInput(0)->GetStreamFormat();
	streamDesc.SetCanonical(kNumInputs, false);
	GetInput(0)->SetStreamFormat(streamDesc);
*/
}

//--------------------------------------------------------------------------------
Leslie::~Leslie()
{
	if (presets != NULL)
		delete[] presets;
	presets = NULL;
}

//--------------------------------------------------------------------------------
OSStatus Leslie::Initialize()
{
	OSStatus status = AUEffectBase::Initialize();

	if (status == noErr)
	{
		hbuf = (float*) malloc(kBufferSize * sizeof(float));
		if (hbuf == NULL)
			status = memFullErr;

		Reset(kAudioUnitScope_Global, (AudioUnitElement)0);

		if ( GetInput(0)->GetStreamFormat().mChannelsPerFrame != GetOutput(0)->GetStreamFormat().mChannelsPerFrame )
		{
#if 0
			if ( IsBypassEffect() )
			{
				SetBypassEffect(false);
				PropertyChanged(kAudioUnitProperty_BypassEffect, kAudioUnitScope_Global, (AudioUnitElement)0);
			}
#endif
			if ( ProcessesInPlace() )
			{
				SetProcessesInPlace(false);
				PropertyChanged(kAudioUnitProperty_InPlaceProcessing, kAudioUnitScope_Global, (AudioUnitElement)0);
			}
		}
	}

	return status;
}

//--------------------------------------------------------------------------------
void Leslie::Cleanup()
{
	if (hbuf != NULL)
		free(hbuf);
	hbuf = NULL;
}

//--------------------------------------------------------------------------------
OSStatus Leslie::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	lspd = 0.0f; hspd = 0.0f;
	lphi = 0.0f; hphi = 1.6f; 

	hpos = 0;
	fbuf1 = fbuf2 = 0.0f;

	if (hbuf != NULL)
		memset(hbuf, 0, kBufferSize * sizeof(float));

	return noErr;
}



#pragma mark -
#pragma mark parameters

//--------------------------------------------------------------------------------
OSStatus Leslie::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	OSStatus status = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_SpeedMode:
			FillInParameterName(outParameterInfo, CFSTR("speed mode"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0;
			outParameterInfo.maxValue = kNumSpeedModes - 1;
			outParameterInfo.defaultValue = kSpeedMode_Fast;
			break;

		case kParam_LowWidth:
			FillInParameterName(outParameterInfo, CFSTR("low width"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 50.0f;
			break;

		case kParam_LowThrob:
			FillInParameterName(outParameterInfo, CFSTR("low throb"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 60.0f;
			break;

		case kParam_HighWidth:
			FillInParameterName(outParameterInfo, CFSTR("high width"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 70.0f;
			break;

		case kParam_HighDepth:
			FillInParameterName(outParameterInfo, CFSTR("high depth"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 60.0f;
			break;

		case kParam_HighThrob:
			FillInParameterName(outParameterInfo, CFSTR("high throb"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 70.0f;
			break;

		case kParam_Crossover:
			FillInParameterName(outParameterInfo, CFSTR("crossover"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = kCrossoverMin;
			outParameterInfo.maxValue = kCrossoverMax;
			outParameterInfo.defaultValue = 450.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayLogarithmic;
			break;

		case kParam_Output:
			FillInParameterName(outParameterInfo, CFSTR("output"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -20.0f;
			outParameterInfo.maxValue = 20.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_Speed:
			FillInParameterName(outParameterInfo, CFSTR("speed"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 200.0f;
			outParameterInfo.defaultValue = 100.0f;
			break;

		default:
			status = kAudioUnitErr_InvalidParameter;
			break;
	}

	return status;
}

//--------------------------------------------------------------------------------
OSStatus Leslie::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_SpeedMode:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("stop|slow|fast"), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}



#pragma mark -
#pragma mark presets

//--------------------------------------------------------------------------------
Leslie::LesliePreset::LesliePreset()
{
	name = CFSTR("Leslie Simulator");
	param = (float*) malloc(kNumParams * sizeof(float));
}

//--------------------------------------------------------------------------------
Leslie::LesliePreset::~LesliePreset()
{
	if (param != NULL)
		free(param);
	param = NULL;
}

//--------------------------------------------------------------------------
OSStatus Leslie::GetPresets(CFArrayRef * outData) const
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
OSStatus Leslie::NewFactoryPresetSet(const AUPreset & inNewFactoryPreset)
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
UInt32 Leslie::SupportedNumChannels(const AUChannelInfo ** outChannelInfo)
{
	static AUChannelInfo plugChannelInfo[] = { {kNumInputs, kNumOutputs}, {kNumOutputs, kNumOutputs} };

	if (outChannelInfo != NULL)
		*outChannelInfo = plugChannelInfo;

	return sizeof(plugChannelInfo) / sizeof(plugChannelInfo[0]);
}

#if 0
//--------------------------------------------------------------------------------
void Leslie::SetBypassEffect(bool inFlag)
{
	if ( GetInput(0)->GetStreamFormat().mChannelsPerFrame == GetOutput(0)->GetStreamFormat().mChannelsPerFrame )
		AUEffectBase::SetBypassEffect(inFlag);
}
#endif

//--------------------------------------------------------------------------------
OSStatus Leslie::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
	// calcs here!
	float ifs = 1.0f / (float)GetSampleRate();
	float spd = kTwoPi * ifs * GetParameter(kParam_Speed)*0.01f;
	float crossover = GetParameter(kParam_Crossover);
	CAAUParameter aup(GetComponentInstance(), kParam_Crossover, kAudioUnitScope_Global, 0);
	crossover = AUParameterValueToLinear(crossover, &aup);
	float filo = 1.0f - powf(10.0f, crossover * (2.27f - 0.54f * crossover) - 1.92f);	// crossover filter coeff

	float hset, hmom, lset, lmom;

	long speedMode = (long) GetParameter(kParam_SpeedMode);
	switch (speedMode)
	{
		case kSpeedMode_Stop:
 			lset = 0.0f; hset = 0.0f;
 			lmom = 0.12f; hmom = 0.1f; 
			break;
		case kSpeedMode_Slow:
 			lset = 0.49f; hset = 0.66f;
 			lmom = 0.27f; hmom = 0.18f;
			break;
		case kSpeedMode_Fast:
		default:
			lset = 5.31f; hset = 6.40f;
			lmom = 0.14f; hmom = 0.09f;
			break;
	}
	hmom = powf(10.0f, -ifs / hmom);
	lmom = powf(10.0f, -ifs / lmom); 
	hset *= spd;
	lset *= spd;

	float gain = 0.4f * powf(10.0f, GetParameter(kParam_Output) / 20.0f);
	float lwid = GetParameter(kParam_LowWidth) * 0.01f;
	lwid *= lwid;
	float llev = GetParameter(kParam_LowThrob) * 0.01f;
	llev = gain * 0.9f * llev * llev;
	float hwid = GetParameter(kParam_HighWidth) * 0.01f;
	hwid *= hwid;
	float hdep = GetParameter(kParam_HighDepth) * 0.01f;
	hdep = hdep * hdep * (float)GetSampleRate() / 760.0f;
	float hlev = GetParameter(kParam_HighThrob) * 0.01f;
	hlev = gain * 0.9f * hlev * hlev;



	const float * in = (float*)(inBuffer.mBuffers[0].mData);
	float * in2 = NULL;
	if (inBuffer.mNumberBuffers > 1)
		in2 = (float*)(inBuffer.mBuffers[1].mData);
	float * out[kNumOutputs];
	for (long i=0; i < kNumOutputs; i++)
		out[i] = (float*)(outBuffer.mBuffers[i].mData);

	float a, c, d, h, l;
	float hl=hlev, ht, hm=hmom, hw=hwid, hd=hdep;
	float ll=llev, lt, lm=lmom, lw=lwid;
	float chp, dchp = 0.0f, clp, dclp = 0.0f, shp, dshp = 0.0f, slp, dslp = 0.0f;
	float hint, k0=0.03125f, k1=32.f;	//k0 = 1/k1
	long	hdd, hdd2, k=0;

	ht = hset * (1.0f - hm);	// target speeds
	lt = lset * (1.0f - lm);

	chp = cosf(hphi); chp *= chp * chp;	// set LFO values
	clp = cosf(lphi);
	shp = sinf(hphi);
	slp = sinf(lphi);

	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		if (in2 != NULL)
			a = in[samp] + in2[samp];	// summed stereo input
		else
			a = in[samp] * 2.0f;	// mono input

		if (k > 0)
			k--;
		else	// linear piecewise approx to LFO waveforms
		{
			lspd = (lm * lspd) + lt;	// tend to required speed
			hspd = (hm * hspd) + ht;
			lphi += k1 * lspd;
			hphi += k1 * hspd;
 													
			dchp = cosf(hphi + k1*hspd);
			dchp = k0 * (dchp * dchp * dchp - chp);	// sin^3 level mod
			dclp = k0 * (cosf(lphi + k1*lspd) - clp);
			dshp = k0 * (sinf(hphi + k1*hspd) - shp);
			dslp = k0 * (sinf(lphi + k1*lspd) - slp);
			
			k = (long)k1;
		}

		fbuf1 = filo * (fbuf1 - a) + a;	// crossover
		fbuf2 = filo * (fbuf2 - fbuf1) + fbuf1;	
		h = (gain - hl * chp) * (a - fbuf2);	// volume
		l = (gain - ll * clp) * fbuf2;

		if (hpos > 0) hpos--; else hpos=200;	// delay input pos
		hint = hpos + hd * (1.0f + chp);	// delay output pos 
		hdd = (int)hint; 
		hint = hint - hdd;	// linear intrpolation
		hdd2 = hdd + 1;
		if (hdd > 199) { if (hdd > 200) hdd -= 201; hdd2 -= 201; }

		hbuf[hpos] = h;	// delay input
		a = hbuf[hdd];
		h += a + hint * (hbuf[hdd2] - a);	// delay output

		c = l + h; 
		d = l + h;
		h *= hw * shp;
		l *= lw * slp;
		d += l - h;
		c += h - l;

		out[0][samp] = c;	// output
		out[1][samp] = d;

		chp += dchp;
		clp += dclp;
		shp += dshp;
		slp += dslp;
	}
	lphi = fmodf(lphi+(k1-k)*lspd, kTwoPi);
	hphi = fmodf(hphi+(k1-k)*hspd, kTwoPi);
	// catch denormals
	if (fabsf(fbuf1) <= 1.0e-10f)
		fbuf1 = 0.0f;
	if (fabsf(fbuf2) <= 1.0e-10f)
		fbuf2 = 0.0f; 

	return noErr;
}
