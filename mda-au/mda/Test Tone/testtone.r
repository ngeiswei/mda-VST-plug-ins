#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        2400
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'Test'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Test Tone"
#define DESCRIPTION   "Audio Test Signal Generator"
#define ENTRY_POINT   "TestToneEntry"

#include "AUResources.r"
