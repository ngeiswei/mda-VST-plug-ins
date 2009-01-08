#!/bin/sh

make clean all

for PLUGIN in mdaAmbience mdaBandisto mdaBeatBox mdaCombo mdaDe-ess mdaDegrade mdaDelay mdaDetune mdaDither mdaDubDelay mdaDX10 mdaDynamics mdaEnvelope mdaEPiano mdaImage mdaJX10 mdaLeslie mdaLimiter mdaLooplex mdaLoudness  mdaMultiBand mdaOverdrive mdaPiano mdaRePsycho! mdaRezFilter mdaRingMod mdaRoundPan mdaShepard mdaSplitter mdaStereo mdaSubSynth mdaTalkBox mdaTestTone mdaThruZero mdaTracker mdaTransient mdaVocInput mdaVocoder
do
  echo Bundling $PLUGIN
  ./makebundle $PLUGIN.vst 1
done

make clean

echo All Done.


