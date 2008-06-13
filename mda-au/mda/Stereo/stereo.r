#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        2100
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'mdaS'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Stereo Simulator"
#define DESCRIPTION   "add artificial width to a mono signal"
#define ENTRY_POINT   "StereoEntry"

#include "AUResources.r"
