// "mda TestTone" v1.0  Copyright(c)2002 Paul Kellett (@mda-vst.com)

#include "AUEffectBase.h"

#include "mda-common.h"


const double TWOPI = 6.283185308;


enum
{
	_MODE,
	_OUTPUT,
	_LEVEL,
	_FREQ,
	_ONTIME,
	_OFFTIME,
	_THRU,
	NPARAM
};

static float gaintab[51] = { 2.434f, 2.409f, 2.39f, 2.372f, 2.341f, 2.304f, 2.268f, 2.226f, 2.179f, 2.132f,
											2.076f, 2.020f, 1.96f, 1.899f, 1.833f, 1.764f, 1.700f, 1.628f, 1.558f, 1.485f,
                     	1.411f, 1.337f, 1.263f, 1.19f, 1.115f, 1.042f, 0.97f,  0.899f, 0.828f, 0.76f,
											0.694f, 0.6304f,0.5685f,0.51f, 0.454f, 0.3994f,0.3505f,0.3025f, 0.26f, 0.219f,
	                    0.183f, 0.15f,  0.12f, 0.0948f,0.0729f,0.0547f,0.0403f, 0.03f, 0.023f, 0.0196f,
											0.0f };

class TestTone : public AUEffectBase
{
public:
	TestTone(AudioComponentInstance component);
	virtual OSStatus Initialize();
	virtual void Cleanup();
	virtual OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual OSStatus GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
	virtual bool SupportsTail() { return true; }
	virtual OSStatus Version() { return PLUGIN_VERSION; }
	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);

protected:
	void thirdOct(float w);
	long  mode, count, countmax;
	unsigned long noise;
	double phs, dphs, ddphs;
	float gain, env;
	float z0, z1, z2, z3, z4, z5, z6, z10, z11, z12, z13;
	float a0, a1, a2, a3, a4, a5, a6;
	float ** in;
	float ** out;
};


COMPONENT_ENTRY(TestTone)


TestTone::TestTone(AudioComponentInstance component) : AUEffectBase(component)
{
	//init internal parameters...
	count = mode = 0;
	phs = dphs = ddphs = 0.0;
	gain = env = 0.0f;
	noise = 22222;

	in = out = NULL;

	// init internal parameters...
	for (AudioUnitParameterID i=0; i < NPARAM; i++)
	{
		AudioUnitParameterInfo paramInfo;
		OSStatus status = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (status == noErr)
			SetParameter(i, paramInfo.defaultValue);
	}
}


OSStatus TestTone::Initialize()
{
	OSStatus status = AUEffectBase::Initialize();

	if (status == noErr)
	{
		in = (float**) malloc(GetNumberOfChannels() * sizeof(float*));
		out = (float**) malloc(GetNumberOfChannels() * sizeof(float*));
		if ( (in == NULL) || (out == NULL) )
			status = memFullErr;
	}

	return status;
}

void TestTone::Cleanup()
{
	AUEffectBase::Cleanup();

	if (in != NULL)
		free(in);
	in = NULL;
	if (out != NULL)
		free(out);
	out = NULL;
}


OSStatus TestTone::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings)
{
  if (inScope != kAudioUnitScope_Global)
	return kAudioUnitErr_InvalidProperty;

	switch (inParameterID)
	{
		case _MODE:
			if (outStrings != NULL)
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("Off|White Noise|Pink Noise|1/3 Octave|Sine Wave|Log Sweep|Lin Sweep"), CFSTR("|"));	
			return noErr;

		case _OUTPUT:
			if (outStrings != NULL)
				*outStrings = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, 
								CFSTR("All Channels|1 - Left|2 - Right|3|4|5|6|7|8"), CFSTR("|"));	
			return noErr;			
	}
	return kAudioUnitErr_InvalidPropertyValue;
}


OSStatus TestTone::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	OSStatus status = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

	switch (inParameterID)
	{
		case _MODE:
			FillInParameterName(outParameterInfo, CFSTR("Mode"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0;
			outParameterInfo.maxValue = 6;
			outParameterInfo.defaultValue = 4;
			break;

		case _OUTPUT:
			FillInParameterName(outParameterInfo, CFSTR("Output"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0;
			outParameterInfo.maxValue = 8;
			outParameterInfo.defaultValue = 0;
			break;
			
		case _LEVEL:
			FillInParameterName(outParameterInfo, CFSTR("Level"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = -80.0;
			outParameterInfo.maxValue = 0.0;
			outParameterInfo.defaultValue = -18.0;
			break;

		case _FREQ:
			FillInParameterName(outParameterInfo, CFSTR("Frequency"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = 20.0;
			outParameterInfo.maxValue = 20000.0;
			outParameterInfo.defaultValue = 997.0;
			outParameterInfo.flags |= kAudioUnitParameterFlag_DisplayLogarithmic;
			break;

		case _ONTIME:
			FillInParameterName(outParameterInfo, CFSTR("On Time"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
			outParameterInfo.minValue = 1.0;
			outParameterInfo.maxValue = 30.0;
			outParameterInfo.defaultValue = 10.0;
			break;

		case _OFFTIME:
			FillInParameterName(outParameterInfo, CFSTR("Off Time"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
			outParameterInfo.minValue = 0.0;
			outParameterInfo.maxValue = 10.0;
			outParameterInfo.defaultValue = 0;
			break;

		case _THRU:
			FillInParameterName(outParameterInfo, CFSTR("Audio Thru"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
			outParameterInfo.minValue = 0;
			outParameterInfo.maxValue = 1;
			outParameterInfo.defaultValue = 1;
			break;

		default:
			status = kAudioUnitErr_InvalidParameter;
			break;
	}
	return status;
}


void TestTone::thirdOct(float w)
{
  float g, rr, fl, fh, ff;
  float a7, aa4, aa1 = 0.2f - 0.05f * w;

  if(w < 0.2f) aa4 = 0.98686f + 0.045f * w * w;
  else aa4 = 0.98266f + 0.023f * w;

  float xx = 100.0f * w;
  long i = (long)xx;
// XXX added by Sophia, otherwise can go out of bounds
  long maxGaintabIndexMinus1 = (sizeof(gaintab) / sizeof(gaintab[0])) - 2;
  if (i > maxGaintabIndexMinus1)
	i = maxGaintabIndexMinus1;
// XXX end of Sophia additions
  xx = xx - (float)i;
  a7 = gaintab[i] + xx * (gaintab[i + 1] - gaintab[i]);
  a7 = a7 + a7 * 0.03f * w;

  ff = 6.28318530718f * w;
  fh = ff * 1.1025f;
  fl = ff / 1.1025f;
  ff = ff * aa4; //tilt

  rr = 1.0f - aa1 * ff;
  a0 = 4.0f * rr * cosf(ff) / (1.0f + rr);
  a1 = -rr;  g = 1.0f - rr;

  rr = 1.0f - 0.1f * fh;
  a3 = 4.0f * rr * cosf(fh) / (1.0f + rr);
  a4 = -rr;  g *= 1.0f - rr;

  rr = 1.0f - 0.1f * fl;
  a5 = 4.0f * rr * cosf(fl) / (1.0f + rr);
  a6 = -rr;  g *= 1.0f - rr;

  g *= 2.529f / sqrtf(2.0f * w);
  if(w > 0.5f || w < 0.0f) a2 = 0.0f;  else a2 = g * a7;
}


OSStatus TestTone::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags,
	const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
  //bool silentInput = (ioActionFlags & kAudioUnitRenderAction_OutputIsSilence) != 0;
  //ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
	//after processing... if not silent, remove IsSilence from ActionFlags 
  //if(!silentInput)
	ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;	

	UInt32 nChannels = outBuffer.mNumberBuffers;

	for (UInt32 i=0; i < nChannels; i++)
	{
		in[i] = (float*) (inBuffer.mBuffers[i].mData);
		out[i] = (float*) (outBuffer.mBuffers[i].mData);
	}

	////////algorithm below here...

	float x, y, thru;
	thru = GetParameter(_THRU);

	double w = GetParameter(_FREQ) / GetSampleRate();
	
	long newmode = (long)GetParameter(_MODE);
	if(newmode != mode || (mode == 3 && w != dphs)) //reset
	{
		mode = newmode;
		count = -2;
		z0 = z1 = z2 = z3 = z4 = z5 = z6 = 0.0f;
		z10 = z11 = z12 = z13 = 0.0f;
		env = 0.0f;
		if(mode == 3) { dphs = w; thirdOct(dphs); }
	}
	if(mode == 4) dphs = w * TWOPI;

	gain = powf(10.0f, 0.05f * GetParameter(_LEVEL));
	UInt32 mask = (UInt32)GetParameter(_OUTPUT);	
	if(mask) mask = 1 << mask; else mask = 0xFFFFFFF;
	
	for (UInt32 samp=0; samp < inFramesToProcess; samp++)
	{
		noise = (noise * 196314165) + 907633515;
		long n = (noise & 0x7FFFFF) + 0x40000000;
		x = 3.0f - *(float *)&n;
		
		switch(mode)
		{
			case 1: //white noise
				y = z0 + 0.5f * x;  z0 = x - 0.5f * y;
				x = z1 + 0.5f * y;  z1 = y - 0.5f * x;
				y = z2 + 0.5f * x;  z2 = x - 0.5f * y;
				x = z3 + 0.5f * y;  z3 = y - 0.5f * x;
				x *= 1.2246f;
        break;
				
			case 2: //pink noise
				z0 = 0.99886f * z0 + x * 0.0222809f;
				z1 = 0.99332f * z1 + x * 0.0301300f;
				z2 = 0.96900f * z2 + x * 0.0617451f;
				z3 = 0.86650f * z3 + x * 0.1246067f;
				z4 = 0.55000f * z4 + x * 0.2138889f;
				z5 = -0.7616f * z5 - x * 0.0067816f;
				y = z0 + z1 + z2 + z3 + z4 + z5 + z6 + x * 0.21519f;
				z6 = x * 0.0465244f;
				x = y;
				break;
				
			case 3: //1/3 octave noise
				y = x - z10 -z10 + z12 + z12 - z13;
				z13 = z12;  z12 = z11;  z11 = z10;  z10 = x;
				y += a0 * z0 + a1 * z1;  z1 = z0;  z0 = y; //6th order 1/3-octave filter
				y += a3 * z3 + a4 * z4;  z4 = z3;  z3 = y;
				y += a5 * z5 + a6 * z6;  z6 = z5;  z5 = y;
				x = a2 * y;
				break;
				
      case 4: //sine
				x = sinf(phs);  phs += dphs;  if(phs > TWOPI) phs -= TWOPI;
				break;

      case 5: //log sweep
				x = sinf(phs);  phs += dphs;  if(phs > TWOPI) phs -= TWOPI;
        dphs *= ddphs;
				break;

			case 6: //lin sweep
				x = sinf(phs);  phs += dphs;  if(phs > TWOPI) phs -= TWOPI;
				dphs += ddphs; 
				break;

			default: x = 0.0f;
		}
    x *= env;

    count++;
    if(count < 0)
		{
      env *= 0.995f;
      if(count == -1)
			{
        float fs = GetSampleRate();
				w = GetParameter(_FREQ);
				ddphs = GetParameter(_ONTIME) * fs;
				countmax = (long)(ddphs);
				phs = 0.0;
        if(mode == 5) //log sweep
				{
				  if(w > 900.0)
					{
					  dphs = TWOPI * 20.0 / fs;
						w = w / 20.0;
					}
					else
					{
            dphs = TWOPI * w / fs;
						w = 20000.0 / w;
					}
					ddphs = exp(log(w) / ddphs);
				}
				else if(mode == 6) //lin sweep
				{
					dphs = 0.0;
					ddphs = TWOPI * w / (ddphs * fs);
				}
			}
		}
		else
		{
			env += 0.005f * (gain - env);
			if(count == countmax)
			{
				count = -(long)(GetParameter(_OFFTIME) * GetSampleRate());
				if(count == 0 && mode > 4) count = -1000; //force sweep restart
			}
		}

		UInt32 m = mask;
		for(UInt32 c=0; c<nChannels; c++)
		{
			y = in[c][samp] * thru;
			m >>= 1; if(m & 1) y += x; //test tone on selected channel
			out[c][samp] = y;
    }
  }
  if(env < 1.0e-6) env = 0.0f; //anti-denormal
		
	return noErr;
}

