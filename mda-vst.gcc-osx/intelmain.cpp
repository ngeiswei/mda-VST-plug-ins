// adds "main_macho" to the exported symbols

#include "pluginterfaces/vst2.x/aeffectx.h"	// Version 2.x 'C' Extensions and Structures

extern "C"
{
	
#define VST_EXPORT   __attribute__ ((visibility ("default")))
	
    extern VST_EXPORT AEffect * VSTPluginMain(audioMasterCallback audioMaster);
	
#if (__APPLE__ && __i386__)
    VST_EXPORT AEffect * main_macho(audioMasterCallback audioMaster)
    {
        return VSTPluginMain(audioMaster);
    }
#endif
}

