#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"


// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        2000
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'mdaX'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Shepard Tones"
#define DESCRIPTION   "continuously rising/falling tone generator"
#define ENTRY_POINT   "ShepardEntry"

#include "AUResources.r"
