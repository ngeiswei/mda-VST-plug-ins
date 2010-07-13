#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"


// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        1700
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'mdaY'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Re-Psycho!"
#define DESCRIPTION   "drum loop pitch changer"
#define ENTRY_POINT   "RePsychoEntry"

#include "AUResources.r"
