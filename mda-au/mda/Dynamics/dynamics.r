#include <AudioUnit/AudioUnit.r>

#include "../mda-common.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define RES_ID        1500
#define COMP_TYPE     kAudioUnitType_Effect
#define COMP_SUBTYPE  'Dyna'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION       PLUGIN_VERSION
#define NAME          PLUGIN_MANUFACTURER_NAME": Dynamics"
#define DESCRIPTION   "Compressor / Limiter / Gate"
#define ENTRY_POINT   "DynamicsEntry"

#include "AUResources.r"
