#include "AUEffectBase.h"

#include "mda-common.h"


//--------------------------------------------------------------------------------
enum
{
	kParam_Width,
	kParam_Mode,
	kParam_Delay,
	kParam_Balance,
	kParam_Mod,
	kParam_Rate,
	kNumParams
};

enum
{
	kMode_Haas,
	kMode_Comb,
	kNumModes
};

const SInt16 kNumInputs = 1;
const SInt16 kNumOutputs = 2;

const double kBufferSize_Seconds = 0.1;


//--------------------------------------------------------------------------------
class Stereo : public AUEffectBase
{
public:
	Stereo(AudioComponentInstance inComponentInstance);

	virtual OSStatus Initialize();
	virtual void Cleanup();
	virtual OSStatus Reset(AudioUnitScope inScope, AudioUnitElement inElement);
	virtual OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual OSStatus GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
	virtual bool SupportsTail()
		{	return true;	}
	virtual UInt32 SupportedNumChannels(const AUChannelInfo ** outChannelInfo);
	virtual OSStatus Version()
		{	return PLUGIN_VERSION;	}
	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);

private:
	float phi;

	float * buffer;
	long bufpos, bufsize;
};



//--------------------------------------------------------------------------------
COMPONENT_ENTRY(Stereo)

//--------------------------------------------------------------------------------
Stereo::Stereo(AudioComponentInstance inComponentInstance)
	: AUEffectBase(inComponentInstance, false)
{
	bufsize = 0;

	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AudioUnitParameterInfo paramInfo;
		OSStatus status = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (status == noErr)
			SetParameter(i, paramInfo.defaultValue);
	}
}

//--------------------------------------------------------------------------------
OSStatus	Stereo::Initialize()
{
	OSStatus status = AUEffectBase::Initialize();

	if (status == noErr)
	{
		bufsize = (long) (kBufferSize_Seconds * GetSampleRate());
		buffer = (float*) malloc(bufsize * sizeof(float));

		const AudioUnitElement elem = 0;
		Reset(kAudioUnitScope_Global, elem);

		if ( GetStreamFormat(kAudioUnitScope_Input, elem).mChannelsPerFrame != GetStreamFormat(kAudioUnitScope_Output, elem).mChannelsPerFrame )
		{
			if ( ProcessesInPlace() )
			{
				SetProcessesInPlace(false);
				PropertyChanged(kAudioUnitProperty_InPlaceProcessing, kAudioUnitScope_Global, elem);
			}
		}
	}

	return status;
}

// still do something sensible with stereo inputs? Haas?

//--------------------------------------------------------------------------------
void Stereo::Cleanup()
{
	if (buffer != NULL)
		free(buffer);
	buffer = NULL;

	AUEffectBase::Cleanup();
}

//--------------------------------------------------------------------------------
OSStatus Stereo::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	OSStatus status = AUEffectBase::Reset(inScope, inElement);

	bufpos = 0;
	phi = 0.0f;
	if (buffer != NULL)
		memset(buffer, 0, bufsize * sizeof(float));

	return status;
}

//--------------------------------------------------------------------------------
OSStatus Stereo::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	OSStatus status = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case kParam_Width:
			FillInParameterName(outParameterInfo, CFSTR("Width"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 56.0f;
			break;

		case kParam_Mode:
			FillInParameterName(outParameterInfo, CFSTR("Mode"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = kNumModes - 1;
			outParameterInfo.defaultValue = kMode_Comb;
			break;

		case kParam_Delay:
			FillInParameterName(outParameterInfo, CFSTR("Delay"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Milliseconds;
			outParameterInfo.minValue = 0.453514739229025f;
			outParameterInfo.maxValue = (kBufferSize_Seconds * 0.5f) * 1000.0f;
			outParameterInfo.defaultValue = 9.17442176870748f;//0.43f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplaySquareRoot;
			break;

		case kParam_Balance:
			FillInParameterName(outParameterInfo, CFSTR("Balance"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Pan;
			outParameterInfo.minValue = -100.0f;
			outParameterInfo.maxValue = 100.0f;
			outParameterInfo.defaultValue = 0.0f;
			break;

		case kParam_Mod:
			FillInParameterName(outParameterInfo, CFSTR("Mod"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Milliseconds;
			outParameterInfo.minValue = 0.0f;
			outParameterInfo.maxValue = (kBufferSize_Seconds * 0.5f) * 1000.0f;
			outParameterInfo.defaultValue = 0.0f;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplaySquareRoot;
			break;

		case kParam_Rate:
			FillInParameterName(outParameterInfo, CFSTR("Rate"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
//powf(10.0f, 2.0f - (3.0f * value))
			outParameterInfo.minValue = 0.01f;//100.0f
			outParameterInfo.maxValue = 10.0f;//0.1f
			outParameterInfo.defaultValue = 0.316227766017f;//3.16227766017f
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayLogarithmic;
			break;

		default:
			status = kAudioUnitErr_InvalidParameter;
			break;
	}

	return status;
}

//--------------------------------------------------------------------------------
OSStatus Stereo::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings)
{
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	switch (inParameterID)
	{
		case kParam_Mode:
			if (outStrings != NULL)
			{
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("Haas|comb"), CFSTR("|"));
				if (*outStrings == NULL)
					return coreFoundationUnknownErr;
			}
			return noErr;
	}

	return kAudioUnitErr_InvalidPropertyValue;
}

//--------------------------------------------------------------------------------
UInt32 Stereo::SupportedNumChannels(const AUChannelInfo ** outChannelInfo)
{
	static AUChannelInfo plugChannelInfo[] = { {kNumInputs, kNumOutputs}, {kNumOutputs, kNumOutputs} };

	if (outChannelInfo != NULL)
		*outChannelInfo = plugChannelInfo;

	return sizeof(plugChannelInfo) / sizeof(plugChannelInfo[0]);
}

//--------------------------------------------------------------------------------
OSStatus Stereo::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags,
	const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
	// update internal parameters...
	float dphi = (float) (3.141 * GetParameter(kParam_Rate) / GetSampleRate());
	float mod = (float) ((double)GetParameter(kParam_Mod) * 0.001 * GetSampleRate());

	long widthMode = (long) GetParameter(kParam_Mode);
	if (widthMode >= kNumModes)
		widthMode = kNumModes - 1;
	else if (widthMode < 0)
		widthMode = 0;
	float widthValue = GetParameter(kParam_Width) * 0.01f;
	float fli, fld, fri, frd;
	if (widthMode == kMode_Haas)
	{
		fli = 1.0f - (widthValue * 0.75f);
		fld = 0.0f;
		fri = 1.0f - widthValue;
		frd = 1.0f - fri;
	}
	else
	{
		fli = ((1.0f - widthValue) * 0.5f) + 0.5f;
		fld = widthValue * 0.5f;
		fri = fli;
		frd = -fld;
	}
	float fdel = (float) ((double)GetParameter(kParam_Delay) * 0.001 * GetSampleRate());
	float balanceValue = GetParameter(kParam_Balance) * 0.01f;
	float balanceScalar = 1.0f - fabsf(balanceValue);
	if (balanceValue > 0.0f)
	{
		fli *= balanceScalar;
		fld *= balanceScalar;
	}
	else
	{
		fri *= balanceScalar;
		frd *= balanceScalar;
	}
	float widthValue_modified = (widthValue * 0.5f) + 0.5f;
	fri *= widthValue_modified;
	frd *= widthValue_modified;
	fli *= widthValue_modified;
	fld *= widthValue_modified;



	const float * in1 = (float*)(inBuffer.mBuffers[0].mData);
	float * in2 = NULL;
	if (inBuffer.mNumberBuffers > 1)
		in2 = (float*)(inBuffer.mBuffers[1].mData);
	float * out[kNumOutputs];
	for (SInt16 i=0; i < kNumOutputs; i++)
		out[i] = (float*)(outBuffer.mBuffers[i].mData);

	if (mod > 0.0f)	// modulated delay
	{
		for (UInt32 samp=0; samp < inFramesToProcess; samp++)
		{
			float a = in1[samp];	// mono input
			if (in2 != NULL)
				a = (a + in2[samp]) * 0.5f;	// summed stereo input

			buffer[bufpos] = a;	// write
			long tmp = (bufpos + (long)(fdel + fabsf(mod * sinf(phi)) ) ) % bufsize;
			float b = buffer[tmp];

			out[0][samp] = (a * fli) - (b * fld);	// output
			out[1][samp] = (a * fri) - (b * frd);

			// buffer position
			bufpos--;
			if (bufpos < 0)
				bufpos = bufsize - 1;

			phi = phi + dphi;
		}
	}
	else
	{
		for (UInt32 samp=0; samp < inFramesToProcess; samp++)
		{
			float a = in1[samp];	// mono input
			if (in2 != NULL)
				a = (a + in2[samp]) * 0.5f;	// summed stereo input

			buffer[bufpos] = a;	// write
			long tmp = (bufpos + (long)(fdel) ) % bufsize;
			float b = buffer[tmp];

			out[0][samp] = (a * fli) - (b * fld);	// output
			out[1][samp] = (a * fri) - (b * frd);

			// buffer position
			bufpos--;
			if (bufpos < 0)
				bufpos = bufsize - 1;
		}
	}
	phi = fmodf(phi, 6.2831853f);

	return noErr;
}
