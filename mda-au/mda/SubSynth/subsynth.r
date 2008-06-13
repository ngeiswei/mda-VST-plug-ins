#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        2200
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'MDAB'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Sub-Bass Synthesizer"
#define DESCRIPTION   "More bass than you could ever need!"
#define ENTRY_POINT   "SubSynthEntry"

#include "AUResources.r"
