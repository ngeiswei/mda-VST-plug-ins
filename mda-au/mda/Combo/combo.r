#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        1000
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'Cmbo'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Combo"
#define DESCRIPTION   "Amp & Speaker Simulator"
#define ENTRY_POINT   "ComboEntry"

#include "AUResources.r"
