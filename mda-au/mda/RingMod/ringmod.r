#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"


// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        1800
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'mdaR'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Ring Mod"
#define DESCRIPTION   "simple ring modulator with sine-wave oscillator"
#define ENTRY_POINT   "RingEntry"

#include "AUResources.r"
