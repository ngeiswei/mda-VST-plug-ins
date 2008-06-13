#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"


// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        1600
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'mdaH'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Leslie"
#define DESCRIPTION   "rotary speaker simulator"
#define ENTRY_POINT   "LeslieEntry"

#include "AUResources.r"
