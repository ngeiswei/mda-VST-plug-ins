// adds "main" to the exported symbols

#include "pluginterfaces/vst2.x/aeffectx.h"	// Version 2.x 'C' Extensions and Structures

extern "C"
{

#define VST_EXPORT   __attribute__ ((visibility ("default")))

    extern VST_EXPORT AEffect * VSTPluginMain(audioMasterCallback audioMaster);

    AEffect * main_plugin (audioMasterCallback audioMaster) asm ("main");

#define main main_plugin

    VST_EXPORT AEffect * main(audioMasterCallback audioMaster)
    {
        return VSTPluginMain(audioMaster);
    }
}

