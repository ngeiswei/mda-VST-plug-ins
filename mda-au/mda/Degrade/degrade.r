#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"


// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        1100
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'mdaC'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Degrade"
#define DESCRIPTION   "sample quality reduction"
#define ENTRY_POINT   "DegradeEntry"

#include "AUResources.r"
