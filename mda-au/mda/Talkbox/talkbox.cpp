// "mda Talkbox" v1.0  Copyright(c)2002 Paul Kellett (@mda-vst.com)
// Based on "SampleEffectUnit" © Copyright 2002 Apple Computer, Inc. All rights reserved.

#include "AUEffectBase.h"

#include "mda-common.h"


//#define BUF_MAX         1600
#define ORD_MAX           50
#define TWO_PI     6.28318530717958647692528676655901f
const double BUF_MAX_SECONDS = 0.01633;
const double MIN_SUPPORTED_SAMPLE_RATE = 8000.0;


enum
{
	_WET,
	_DRY,
	_QUAL,
	_SWAP,
	NPARAM
};


class TalkboxDSP
{
public:
	TalkboxDSP(AUEffectBase * inAudioUnit);
	~TalkboxDSP();

	void Reset();
	void Process(const float * in1, const float * in2, float * out, UInt32 inFramesToProcess);

	Float64 GetSampleRate()
		{	return mAudioUnit->GetSampleRate();	}
	Float32 GetParameter(AudioUnitParameterID inParameterID)
		{	return mAudioUnit->GetParameter(inParameterID);	}

private:
	AUEffectBase * mAudioUnit;

	float * car0, * car1;
	float * window;
	float * buf0, * buf1;
	long BUF_MAX;

	float emphasis;
	long K, N, O, pos;
	float wet, dry, FX;

	float d0, d1, d2, d3, d4;
	float u0, u1, u2, u3, u4;	
};


class Talkbox : public AUEffectBase
{
public:
	Talkbox(AudioUnit component);
	virtual void PostConstructor();
	virtual ComponentResult Initialize();
	virtual void Cleanup();
	virtual ComponentResult Reset(AudioUnitScope inScope, AudioUnitElement inElement);

	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo &outParameterInfo);
	virtual Float64 GetLatency() { return BUF_MAX_SECONDS; }
	virtual bool SupportsTail() { return true; }
	virtual ComponentResult Version() { return PLUGIN_VERSION; }
	virtual ComponentResult Render(AudioUnitRenderActionFlags & ioActionFlags, const AudioTimeStamp & inTimeStamp, UInt32 inFramesToProcess);

private:
	TalkboxDSP ** dspKernels;
	UInt32 numAllocatedChannels;
};


void lpc(float *buf, float *car, long n, long o);
void lpc_durbin(float *r, int p, float *k, float *g);



#pragma mark -

COMPONENT_ENTRY(Talkbox)


Talkbox::Talkbox(AudioUnit component) : AUEffectBase(component, true)
{
	// init internal parameters...
	for (AudioUnitParameterID i=0; i < NPARAM; i++)
	{
		AudioUnitParameterInfo paramInfo;
		ComponentResult result = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (result == noErr)
			SetParameter(i, paramInfo.defaultValue);
	}

	dspKernels = NULL;
	numAllocatedChannels = 0;
}

void Talkbox::PostConstructor()
{
	AUEffectBase::PostConstructor();

	Inputs().Initialize(this, kAudioUnitScope_Input, 2);
	Inputs().SafeGetElement(0)->SetName(CFSTR("modulator"));
	Inputs().SafeGetElement(1)->SetName(CFSTR("carrier"));
}


TalkboxDSP::TalkboxDSP(AUEffectBase * inAudioUnit)
:	mAudioUnit(inAudioUnit)
{
	double fs = GetSampleRate();
	if (fs < MIN_SUPPORTED_SAMPLE_RATE)
		fs = MIN_SUPPORTED_SAMPLE_RATE;
	BUF_MAX = (long) (BUF_MAX_SECONDS * fs);

	buf0   = new float[BUF_MAX];
	buf1   = new float[BUF_MAX];
	window = new float[BUF_MAX];
	car0   = new float[BUF_MAX];
	car1   = new float[BUF_MAX];

	N = 1; //trigger window recalc

	Reset();
}

TalkboxDSP::~TalkboxDSP()
{
	if (buf0 != NULL)
		delete buf0;
	buf0 = NULL;
	if (buf1 != NULL)
		delete buf1;
	buf1 = NULL;
	if (window != NULL)
		delete window;
	window = NULL;
	if (car0 != NULL)
		delete car0;
	car0 = NULL;
	if (car1 != NULL)
		delete car1;
	car1 = NULL;
}

void TalkboxDSP::Reset()
{
	// reset state
	pos = K = 0;
	emphasis = 0.0f;
	FX = 0.0f;

	u0 = u1 = u2 = u3 = u4 = 0.0f;
	d0 = d1 = d2 = d3 = d4 = 0.0f;

	if (buf0 != NULL)
		memset(buf0, 0, BUF_MAX * sizeof(float));
	if (buf1 != NULL)
		memset(buf1, 0, BUF_MAX * sizeof(float));
	if (window != NULL)
		memset(window, 0, BUF_MAX * sizeof(float));
	if (car0 != NULL)
		memset(car0, 0, BUF_MAX * sizeof(float));
	if (car1 != NULL)
		memset(car1, 0, BUF_MAX * sizeof(float));
}


ComponentResult Talkbox::Initialize()
{
	ComponentResult result = AUEffectBase::Initialize();

	if (result == noErr)
	{
		numAllocatedChannels = GetNumberOfChannels();
		dspKernels = (TalkboxDSP**) malloc(numAllocatedChannels * sizeof(TalkboxDSP*));
		for (UInt32 i=0; i < numAllocatedChannels; i++)
			dspKernels[i] = new TalkboxDSP(this);
	}

	return result;
}

void Talkbox::Cleanup()
{
	AUEffectBase::Cleanup();

	if (dspKernels != NULL)
	{
		for (UInt32 i=0; i < numAllocatedChannels; i++)
		{
			if (dspKernels[i] != NULL)
				delete dspKernels[i];
			dspKernels[i] = NULL;
		}
		free(dspKernels);
	}
	dspKernels = NULL;
}

ComponentResult Talkbox::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	ComponentResult result = AUEffectBase::Reset(inScope, inElement);

	if (result == noErr)
	{
		if (dspKernels != NULL)
		{
			for (UInt32 i=0; i < numAllocatedChannels; i++)
			{
				if (dspKernels[i] != NULL)
					dspKernels[i]->Reset();
			}
		}
	}

	return result;
}


ComponentResult Talkbox::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo &outParameterInfo)
{
	ComponentResult result = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;
	
	switch(inParameterID)
	{
		case _WET:
			FillInParameterName(outParameterInfo, CFSTR("Wet"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0;
			outParameterInfo.maxValue = 100.0;
			outParameterInfo.defaultValue = 50.0;
			break;

		case _DRY:
			FillInParameterName(outParameterInfo, CFSTR("Dry"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 0.0;
			outParameterInfo.maxValue = 100.0;
			outParameterInfo.defaultValue = 0.0;
			break;

		case _SWAP:
			FillInParameterName(outParameterInfo, CFSTR("Input Swap"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
			outParameterInfo.minValue = 0.0;
			outParameterInfo.maxValue = 1.0;
			outParameterInfo.defaultValue = 0.0;
			break;

		case _QUAL:
			FillInParameterName(outParameterInfo, CFSTR("Quality"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
			outParameterInfo.minValue = 10.0;
			outParameterInfo.maxValue = 100.0;
			outParameterInfo.defaultValue = 100.0;
			break;
			
		default:
			result = kAudioUnitErr_InvalidParameter;
			break;
	}
	return result;
}


ComponentResult Talkbox::Render(AudioUnitRenderActionFlags & ioActionFlags, const AudioTimeStamp & inTimeStamp, UInt32 inFramesToProcess)
{
	if (!HasInput(0))
{
//fprintf(stderr, "no input bus 0 connection\n");
		return kAudioUnitErr_NoConnection;
}

	ComponentResult result = noErr;
	AUOutputElement *theOutput = GetOutput(0);	// throws if error

	AUInputElement *theInput = GetInput(0);
	result = theInput->PullInput(ioActionFlags, inTimeStamp, 0 /* element */, inFramesToProcess);
	if (result != noErr)
		return result;
	
	if (result == noErr)
	{
		if ( ProcessesInPlace() )
		{
			theOutput->SetBufferList(theInput->GetBufferList() );
		}

		AUInputElement * theInput2 = NULL;
		if (HasInput(1))
		{
			theInput2 = GetInput(1);
			result = theInput2->PullInput(ioActionFlags, inTimeStamp, 1 /* element */, inFramesToProcess);
//if (result != noErr) fprintf(stderr, "PullInput(bus 1) error %ld\n", result);
		}
		else
{
//fprintf(stderr, "no input bus 1 connection\n");
			result = kAudioUnitErr_NoConnection;
}

		if ( ShouldBypassEffect() || (result != noErr) )
		{
			result = noErr;
			// leave silence bit alone
			if (! ProcessesInPlace() )
			{
				theInput->CopyBufferContentsTo(theOutput->GetBufferList());
			}
		}
		else
		{
			// this will read/write silence bit
			for (UInt32 i=0; i < numAllocatedChannels; i++)
			{
				dspKernels[i]->Process((const float*)(theInput->GetBufferList().mBuffers[i].mData), 
										(const float*)(theInput2->GetBufferList().mBuffers[i].mData), 
										(float*)(theOutput->GetBufferList().mBuffers[i].mData), 
										inFramesToProcess);
			}
		}
	}

	return result;
}

void TalkboxDSP::Process(const float * in1, const float * in2, float * out, UInt32 inFramesToProcess)
{
	UInt32 nFrames = inFramesToProcess;

	//algorithm starts here...
	float fs = GetSampleRate();
	if(fs <  MIN_SUPPORTED_SAMPLE_RATE) fs =  MIN_SUPPORTED_SAMPLE_RATE;
//	else if(fs > 96000.0f) fs = 96000.0f;
	long n = BUF_MAX;
	O = (long)((0.0001f + 0.000004f * GetParameter(_QUAL)) * fs);

	if(GetParameter(_SWAP) > 0.5f) { const float *tmp = in1;  in1 = in2;  in2 = tmp; } 
	
	if(n != N) //recalc hanning window
	{
		N = n;
		float dp = TWO_PI / (float)N;
		float p = 0.0f;
		for(n=0; n<N; n++)
		{
			window[n] = 0.5f - 0.5f * (float)cos(p);
			p += dp;
		}
	}

	float wet = 0.00707f * GetParameter(_WET);  wet *= wet;
	float dry = 0.01410f * GetParameter(_DRY);  dry *= dry;
	
	long  p0=pos, p1 = (pos + N/2) % N;
	float e=emphasis, w, o, x, dr, fx=FX;
	float p, q, h0=0.3f, h1=0.77f;
	
	while(nFrames-- > 0)
	{
		o = *in1;  in1++;
		x = *in2;  in2++;
		dr = o;

		p = d0 + h0 *  x; d0 = d1;  d1 = x  - h0 * p;
		q = d2 + h1 * d4; d2 = d3;  d3 = d4 - h1 * q;
		d4 = x;
		x = p + q;

		if(K++)
		{
			 K = 0;
			 car0[p0] = car1[p1] = x; //carrier input
			 x = o - e;  e = o;  //6dB/oct pre-emphasis

	 		 w = window[p0]; fx = buf0[p0] * w;  buf0[p0] = x * w;  //50% overlapping hanning windows
			 if(++p0 >= N) { lpc(buf0, car0, N, O);  p0 = 0; }

			 w = 1.0f - w;  fx += buf1[p1] * w;  buf1[p1] = x * w;
			 if(++p1 >= N) { lpc(buf1, car1, N, O);  p1 = 0; }
		}

		p = u0 + h0 * fx; u0 = u1;  u1 = fx - h0 * p;
		q = u2 + h1 * u4; u2 = u3;  u3 = u4 - h1 * q;  
		u4 = fx;
		x = p + q;

		o = wet * x + dry * dr;
		*out = o;  out++;
	}
	emphasis = e;
	pos = p0;
	FX = fx;
}


void lpc(float *buf, float *car, long n, long o)
{
	float z[ORD_MAX], r[ORD_MAX], k[ORD_MAX], G, x;
	long i, j, nn=n;

	for(j=0; j<=o; j++, nn--)  //buf[] is already emphasized and windowed
	{
		z[j] = r[j] = 0.0f;
		for(i=0; i<nn; i++) r[j] += buf[i] * buf[i+j]; //autocorrelation
	}
	r[0] *= 1.001f;  //stability fix //1.0001f

	if(r[0] < 0.00001f) { for(i=0; i<n; i++) buf[i] = 0.0f; return; } 

	lpc_durbin(r, o, k, &G);  //calc reflection coeffs

	for(i=0; i<=o; i++) 
	{
		if(k[i] > 0.995f) k[i] = 0.995f; else if(k[i] < -0.995f) k[i] = -.995f; ///0.995f;
	}

	for(i=0; i<n; i++)
	{
		x = G * car[i];
		for(j=o; j>0; j--)  //lattice filter
		{ 
			x -= k[j] * z[j-1];
			z[j] = z[j-1] + k[j] * x;
		}
		buf[i] = z[0] = x;  //output buf[] will be windowed elsewhere
	}
}


void lpc_durbin(float *r, int p, float *k, float *g)
{
	int i, j;
	float a[ORD_MAX], at[ORD_MAX], e=r[0];

	for(i=0; i<=p; i++) a[i] = at[i] = 0.0f; //probably don't need to clear at[] or k[]

	for(i=1; i<=p; i++) 
	{
		k[i] = -r[i];

		for(j=1; j<i; j++) 
		{ 
			at[j] = a[j];
			k[i] -= a[j] * r[i-j]; 
		}
		if(fabsf(e) < 1.0e-20f) { e = 0.0f;  break; }
		k[i] /= e;

		a[i] = k[i];
		for(j=1; j<i; j++) a[j] = at[j] + k[i] * at[i-j];

		e *= 1.0f - k[i] * k[i];
	}

	if(e < 1.0e-20f) e = 0.0f;
	*g = (float)sqrt(e);
}

