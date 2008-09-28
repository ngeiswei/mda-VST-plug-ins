---------------------------------------------------------------------------------------
For the compile to succeed under Linux, a change has to be made to the aeffect.h
file in the VST SDK sources.
This will only affect the compiler behaviour under Linux and won't have any effect for
other targets.

// ORIGINAL @ line 37

#elif defined(__GNUC__)
	#pragma pack(push,8)
    	#define VSTCALLBACK __cdecl



// REPLACE @ line 37

#elif defined(__GNUC__)
	#pragma pack(push,8)
	#if defined(__linux__)
		#define VSTCALLBACK
	#else
    		#define VSTCALLBACK __cdecl
    	#endif


---------------------------------------------------------------------------------------
