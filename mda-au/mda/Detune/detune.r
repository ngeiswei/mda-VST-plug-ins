#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"


// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        1200
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'mdat'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Detune"
#define DESCRIPTION   "simple stereo up/down pitch shifting thickener"
#define ENTRY_POINT   "DetuneEntry"

#include "AUResources.r"
