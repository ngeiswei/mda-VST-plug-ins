#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"


// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        1400
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'mdaﬂ'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Dub Delay"
#define DESCRIPTION   "delay with feedback saturation and time/pitch modulation"
#define ENTRY_POINT   "DubDelayEntry"

#include "AUResources.r"
