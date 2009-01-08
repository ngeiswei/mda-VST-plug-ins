#include "mdaEnvelope.h"
#include <math.h>
#include <float.h>

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
  return new mdaEnvelope(audioMaster);
}

mdaEnvelope::mdaEnvelope(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, 4)	// programs, parameters
{
  fParam1 = 0.00f; //mode
  fParam2 = 0.25f; //attack
  fParam3 = 0.40f; //release
  fParam4 = 0.50f; //output

  setNumInputs(2);		  
	setNumOutputs(2);		  
	setUniqueID('mdae');    // identify here
	DECLARE_VST_DEPRECATED(canMono) ();				      
	canProcessReplacing();	
	strcpy(programName, "VCA / Envelope Follower ");
	
  env=0.f; drel=0.f;
  setParameter(0, 0.0f); //go and set initial values!
}


void mdaEnvelope::setParameter(VstInt32 index, float value)
{
  
  switch(index)
  {
    case 0: fParam1 = value; break;
    case 1: fParam2 = value; break;
    case 2: fParam3 = value; break;
    case 3: fParam4 = value; break;
  }
  //calcs here
  mode = int(fParam1*2.9);
  att = (float)pow(10.f, -0.002f - 4.f * fParam2);       
  rel = 1.f - (float)pow(10.f, -2.f - 3.f * fParam3);
  out = (float)pow(10.f,2.f*fParam4-1.f);
  if(mode) out*=0.5f; else out*=2.0f; //constant gain across modes
}


mdaEnvelope::~mdaEnvelope()
{
}


void mdaEnvelope::suspend()
{
  env = drel = 0.f;
}


bool mdaEnvelope::getProductString(char* text) { strcpy(text, "mda Envelope"); return true; }
bool mdaEnvelope::getVendorString(char* text)  { strcpy(text, "mda"); return true; }
bool mdaEnvelope::getEffectName(char* name)    { strcpy(name, "Envelope"); return true; }


void mdaEnvelope::setProgramName(char *name)
{
	strcpy(programName, name);
}


void mdaEnvelope::getProgramName(char *name)
{
	strcpy(name, programName);
}

bool mdaEnvelope::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* name)
{
	if (index == 0) 
	{
	    strcpy(name, programName);
	    return true;
	}
	return false;
}

float mdaEnvelope::getParameter(VstInt32 index)
{
	float v=0;

  switch(index)
  {
    case 0: v = fParam1; break;
    case 1: v = fParam2; break;
    case 2: v = fParam3; break;
    case 3: v = fParam4; break;
  }
  return v;
}


void mdaEnvelope::getParameterName(VstInt32 index, char *label)
{
	switch(index)
  {
    case 0: strcpy(label, " Output:"); break;
    case 1: strcpy(label, " Attack "); break;
    case 2: strcpy(label, " Release"); break;
    case 3: strcpy(label, " Gain   "); break;
  }
}


#include <stdio.h>
void int2strng(VstInt32 value, char *string) { sprintf(string, "%d", value); }
void float2strng(float value, char *string) { sprintf(string, "%.2f", value); }

void mdaEnvelope::getParameterDisplay(VstInt32 index, char *text)
{
	switch(index)
  {
    case 0: switch(mode)
            { 
              case 0: strcpy(text, " L x |R|"); break;
              case 1: strcpy(text, " IN/ENV "); break;
              case 2: strcpy(text, "FLAT/ENV"); break;
            } break;
    case 1: int2strng((long)(float)pow(10.0f, 3.0f*fParam2), text); break;
    case 2: int2strng((long)(float)pow(10.0f, 4.0f*fParam3), text); break;
    case 3: int2strng((long)(40 * fParam4 - 20), text); break;
  } //attack & release times not accurate - included to make user happy!
}


void mdaEnvelope::getParameterLabel(VstInt32 index, char *label)
{
	switch(index)
  {
    case 0: strcpy(label, "        "); break;
    case 1: strcpy(label, "    ms  "); break;
    case 2: strcpy(label, "    ms  "); break;
    case 3: strcpy(label, "    dB  "); break;
  }
}

//--------------------------------------------------------------------------------
// process

void mdaEnvelope::process(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	float *in1 = inputs[0];
	float *in2 = inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];
	float a, b, c, d, x;
  float e=env, at=att, re=rel, dre=drel, db=out;	
  long flat;

  if(dre>(1.0f-re)) dre=(1.0f-re); //can be unstable if re edited
  flat=(mode==2)? 1 : 0; //flatten output audio?

  --in1;	
	--in2;	
	--out1;
	--out2;
	
  if(mode) //Envelope follower mode
  {
    while(--sampleFrames >= 0)
	  {
		  b = *++in1 + *++in2;
  		c = out1[1];
  		d = out2[1]; 

      x = (b<0.f)? -b : b; //rectify

      if(x>e)
      {
        e += at * (x - e); //attack + reset rate
        dre = 1.0f - re;
      }
      else
      {
        e *= (re + dre);  //release
        dre *= 0.9999f;   //increase release rate
      }

      x = db*b;          //VCA
      if(flat)
      {
        if(e<0.01f) x*= 100; else x/=e;  //flatten audio signal
      }
      
      c += 0.5f * e;   //envelope out
      d += x;          //audio out

      *++out1 = c;  
		  *++out2 = d;   
	  }
  }
  else //VCA mode
  {
    while(--sampleFrames >= 0)
	  {
		  a = *++in1; 
      b = *++in2;
	  	c = out1[1];
	  	d = out2[1]; 

      x = (b<0.f)? -b : b; //rectify

      if(x>e)
      {
        e += at * (x - e); //attack + reset rate
        dre = 1.0f - re;
      }
      else
      {
        e *= (re + dre);  //release
        dre *= 0.9999f;   //increase release rate
      }

      x = db * a * e;     //VCA
      c += x;
      d += x;
      
      *++out1 = c;	
		  *++out2 = d;
	  }
  }
  if(fabs(e  )>1.0e-10) env=e;    else env=0.0f;
  if(fabs(dre)>1.0e-10) drel=dre; else drel=0.0f;
}


void mdaEnvelope::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	float *in1 = inputs[0];
	float *in2 = inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];
	float a, b, x;
  float e=env, at=att, re=rel, dre=drel, db=out;	
  long flat;

  if(dre>(1.0f-re)) dre=(1.0f-re); //can be unstable if re edited
  flat=(mode==2)? 1 : 0; //flatten output audio?

	--in1;	
	--in2;	
	--out1;
	--out2;

  if(mode) //Envelope follower mode
  {
    while(--sampleFrames >= 0)
	  {
		  b = *++in1 + *++in2;
      x = (b<0.f)? -b : b; //rectify

      if(x>e)
      {
        e += at * (x - e); //attack + reset rate
        dre = 1.0f - re;
      }
      else
      {
        e *= (re + dre);  //release
        dre *= 0.9999f;   //increase release rate
      }

      x = db*b;  //VCA
      if(flat)
      {
        if(e<0.01f) x*= 100; else x/=e; //flatten audio signal
      }
       
		  *++out2 = 0.5f * e;   //envelope out
      *++out1 = x;	        //audio out
	  }
  }
  else //VCA mode
  {
    while(--sampleFrames >= 0)
	  {
		  a = *++in1; 
      b = *++in2;

      x = (b<0.f)? -b : b; //rectify

      if(x>e)
      {
        e += at * (x - e); //attack + reset rate
        dre = 1.0f - re;
      }
      else
      {
        e *= (re + dre);  //release
        dre *= 0.9999f;   //increase release rate
      }

      x = db * a * e;     //VCA
 
      *++out1 = x;	
		  *++out2 = x;
	  }
  }
  if(fabs(e  )>1.0e-10) env=e;    else env=0.0f;
  if(fabs(dre)>1.0e-10) drel=dre; else drel=0.0f;
}
