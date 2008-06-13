#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"


// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        2500
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'mdaJ'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Tracker"
#define DESCRIPTION   "pitch-tracking oscillator, or pitch-tracking EQ"
#define ENTRY_POINT   "TrackerEntry"

#include "AUResources.r"
