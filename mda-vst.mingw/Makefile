#========================================
# S e t t i n g s
#========================================
CC = g++
DYN = .dll
EXT = 
SRC = ../src/
VST = ../vst/
SDK = $(VST)public.sdk/source/vst2.x/
FFLAGS = -fmessage-length=0 -finline-functions -fno-exceptions -fno-rtti
MFLAGS = -mmmx
CFLAGS = -Wall -Os -s -c $(MFLAGS) $(FFLAGS) -I$(VST) -I$(SDK)
LFLAGS = -Wall -s -shared -static exports.def

#========================================
# O b j e c t s
#========================================
# (common)
#----------------------------------------
OBJM = vstplugmain
OBJA = audioeffect
OBJX = audioeffectx
OBJC = $(OBJM).o $(OBJA).o $(OBJX).o

$(OBJM).o : $(SDK)$(OBJM).cpp
	$(CC) $(CFLAGS) $(SDK)$(OBJM).cpp
	
$(OBJX).o : $(SDK)$(OBJX).cpp
	$(CC) $(CFLAGS) $(SDK)$(OBJX).cpp
	
$(OBJA).o : $(SDK)$(OBJA).cpp
	$(CC) $(CFLAGS) $(SDK)$(OBJA).cpp
#----------------------------------------
# (output)
#----------------------------------------
OBJE = mdaAmbience mdaBandisto mdaBeatBox mdaCombo mdaDe-ess mdaDegrade mdaDelay mdaDetune mdaDither mdaDubDelay mdaDX10 mdaDynamics mdaEnvelope mdaEPiano mdaImage mdaJX10 mdaLeslie mdaLimiter mdaLooplex mdaLoudness  mdaMultiBand mdaOverdrive mdaPiano mdaRePsycho! mdaRezFilter mdaRingMod mdaRoundPan mdaShepard mdaSplitter mdaStereo mdaSubSynth mdaTalkBox mdaTestTone mdaThruZero mdaTracker mdaTransient mdaVocInput mdaVocoder
	 
OBJD = $(OBJE:=$(DYN))

#========================================
# B u i l d
#========================================
# all / clean
#----------------------------------------
all: $(OBJE)

clean:
	rm -f *.o $(OBJD)
	
cleanobj:
	rm -f *.o
#----------------------------------------
# individual
#----------------------------------------
mdaAmbience: $(OBJC) mdaAmbience.o
	$(CC) $(LFLAGS) $(OBJC) mdaAmbience.o -o mdaAmbience$(DYN)
mdaAmbience.o: $(SRC)mdaAmbience.cpp $(SRC)mdaAmbience.h
	$(CC) $(CFLAGS) $(SRC)mdaAmbience.cpp
#----------------------------------------
mdaBandisto: $(OBJC) mdaBandisto.o
	$(CC) $(LFLAGS) $(OBJC) mdaBandisto.o -o mdaBandisto$(DYN)
mdaBandisto.o: $(SRC)mdaBandisto.cpp $(SRC)mdaBandisto.h
	$(CC) $(CFLAGS) $(SRC)mdaBandisto.cpp
#----------------------------------------
mdaBeatBox: $(OBJC) mdaBeatBox.o
	$(CC) $(LFLAGS) $(OBJC) mdaBeatBox.o -o mdaBeatBox$(DYN)
mdaBeatBox.o: $(SRC)mdaBeatBox.cpp $(SRC)mdaBeatBox.h
	$(CC) $(CFLAGS) $(SRC)mdaBeatBox.cpp
#----------------------------------------
mdaCombo: $(OBJC) mdaCombo.o
	$(CC) $(LFLAGS) $(OBJC) mdaCombo.o -o mdaCombo$(DYN)
mdaCombo.o: $(SRC)mdaCombo.cpp $(SRC)mdaCombo.h
	$(CC) $(CFLAGS) $(SRC)mdaCombo.cpp
#----------------------------------------
mdaDe-ess: $(OBJC) mdaDe-ess.o
	$(CC) $(LFLAGS) $(OBJC) mdaDe-ess.o -o mdaDe-ess$(DYN)
mdaDe-ess.o: $(SRC)mdaDe-ess.cpp $(SRC)mdaDe-ess.h
	$(CC) $(CFLAGS) $(SRC)mdaDe-ess.cpp
#----------------------------------------
mdaDegrade: $(OBJC) mdaDegrade.o
	$(CC) $(LFLAGS) $(OBJC) mdaDegrade.o -o mdaDegrade$(DYN)
mdaDegrade.o: $(SRC)mdaDegrade.cpp $(SRC)mdaDegrade.h
	$(CC) $(CFLAGS) $(SRC)mdaDegrade.cpp
#----------------------------------------
mdaDelay: $(OBJC) mdaDelay.o
	$(CC) $(LFLAGS) $(OBJC) mdaDelay.o -o mdaDelay$(DYN)
mdaDelay.o: $(SRC)mdaDelay.cpp $(SRC)mdaDelay.h
	$(CC) $(CFLAGS) $(SRC)mdaDelay.cpp
#----------------------------------------
mdaDetune: $(OBJC) mdaDetune.o
	$(CC) $(LFLAGS) $(OBJC) mdaDetune.o -o mdaDetune$(DYN)
mdaDetune.o: $(SRC)mdaDetune.cpp $(SRC)mdaDetune.h
	$(CC) $(CFLAGS) $(SRC)mdaDetune.cpp
#----------------------------------------
mdaDither: $(OBJC) mdaDither.o
	$(CC) $(LFLAGS) $(OBJC) mdaDither.o -o mdaDither$(DYN)
mdaDither.o: $(SRC)mdaDither.cpp $(SRC)mdaDither.h
	$(CC) $(CFLAGS) $(SRC)mdaDither.cpp
#----------------------------------------
mdaDubDelay: $(OBJC) mdaDubDelay.o
	$(CC) $(LFLAGS) $(OBJC) mdaDubDelay.o -o mdaDubDelay$(DYN)
mdaDubDelay.o: $(SRC)mdaDubDelay.cpp $(SRC)mdaDubDelay.h
	$(CC) $(CFLAGS) $(SRC)mdaDubDelay.cpp
#----------------------------------------
mdaDX10: $(OBJC) mdaDX10.o
	$(CC) $(LFLAGS) $(OBJC) mdaDX10.o -o mdaDX10$(DYN)
mdaDX10.o: $(SRC)mdaDX10.cpp $(SRC)mdaDX10.h
	$(CC) $(CFLAGS) $(SRC)mdaDX10.cpp
#----------------------------------------
mdaDynamics: $(OBJC) mdaDynamics.o
	$(CC) $(LFLAGS) $(OBJC) mdaDynamics.o -o mdaDynamics$(DYN)
mdaDynamics.o: $(SRC)mdaDynamics.cpp $(SRC)mdaDynamics.h
	$(CC) $(CFLAGS) $(SRC)mdaDynamics.cpp
#----------------------------------------
mdaEPiano: $(OBJC) mdaEPiano.o
	$(CC) $(LFLAGS) $(OBJC) mdaEPiano.o -o mdaEPiano$(DYN)
mdaEPiano.o: $(SRC)mdaEPiano.cpp $(SRC)mdaEPiano.h
	$(CC) $(CFLAGS) $(SRC)mdaEPiano.cpp
#----------------------------------------
mdaEnvelope: $(OBJC) mdaEnvelope.o
	$(CC) $(LFLAGS) $(OBJC) mdaEnvelope.o -o mdaEnvelope$(DYN)
mdaEnvelope.o: $(SRC)mdaEnvelope.cpp $(SRC)mdaEnvelope.h
	$(CC) $(CFLAGS) $(SRC)mdaEnvelope.cpp
#----------------------------------------
mdaImage: $(OBJC) mdaImage.o
	$(CC) $(LFLAGS) $(OBJC) mdaImage.o -o mdaImage$(DYN)
mdaImage.o: $(SRC)mdaImage.cpp $(SRC)mdaImage.h
	$(CC) $(CFLAGS) $(SRC)mdaImage.cpp
#----------------------------------------
mdaJX10: $(OBJC) mdaJX10.o
	$(CC) $(LFLAGS) $(OBJC) mdaJX10.o -o mdaJX10$(DYN)
mdaJX10.o: $(SRC)mdaJX10.cpp $(SRC)mdaJX10.h
	$(CC) $(CFLAGS) $(SRC)mdaJX10.cpp
#----------------------------------------
mdaLeslie: $(OBJC) mdaLeslie.o
	$(CC) $(LFLAGS) $(OBJC) mdaLeslie.o -o mdaLeslie$(DYN)
mdaLeslie.o: $(SRC)mdaLeslie.cpp $(SRC)mdaLeslie.h
	$(CC) $(CFLAGS) $(SRC)mdaLeslie.cpp
#----------------------------------------
mdaLimiter: $(OBJC) mdaLimiter.o
	$(CC) $(LFLAGS) $(OBJC) mdaLimiter.o -o mdaLimiter$(DYN)
mdaLimiter.o: $(SRC)mdaLimiter.cpp $(SRC)mdaLimiter.h
	$(CC) $(CFLAGS) $(SRC)mdaLimiter.cpp
#----------------------------------------
mdaLooplex: $(OBJC) mdaLooplex.o
	$(CC) $(LFLAGS) $(OBJC) mdaLooplex.o -o mdaLooplex$(DYN)
mdaLooplex.o: $(SRC)mdaLooplex.cpp $(SRC)mdaLooplex.h
	$(CC) $(CFLAGS) $(SRC)mdaLooplex.cpp
#----------------------------------------
mdaLoudness: $(OBJC) mdaLoudness.o
	$(CC) $(LFLAGS) $(OBJC) mdaLoudness.o -o mdaLoudness$(DYN)
mdaLoudness.o: $(SRC)mdaLoudness.cpp $(SRC)mdaLoudness.h
	$(CC) $(CFLAGS) $(SRC)mdaLoudness.cpp
#----------------------------------------
mdaMultiBand: $(OBJC) mdaMultiBand.o
	$(CC) $(LFLAGS) $(OBJC) mdaMultiBand.o -o mdaMultiBand$(DYN)
mdaMultiBand.o: $(SRC)mdaMultiBand.cpp $(SRC)mdaMultiBand.h
	$(CC) $(CFLAGS) $(SRC)mdaMultiBand.cpp
#----------------------------------------
mdaOverdrive: $(OBJC) mdaOverdrive.o
	$(CC) $(LFLAGS) $(OBJC) mdaOverdrive.o -o mdaOverdrive$(DYN)
mdaOverdrive.o: $(SRC)mdaOverdrive.cpp $(SRC)mdaOverdrive.h
	$(CC) $(CFLAGS) $(SRC)mdaOverdrive.cpp
#----------------------------------------
mdaPiano: $(OBJC) mdaPiano.o
	$(CC) $(LFLAGS) $(OBJC) mdaPiano.o -o mdaPiano$(DYN)
mdaPiano.o: $(SRC)mdaPiano.cpp $(SRC)mdaPiano.h
	$(CC) $(CFLAGS) $(SRC)mdaPiano.cpp
#----------------------------------------
mdaRePsycho!: $(OBJC) mdaRePsycho!.o
	$(CC) $(LFLAGS) $(OBJC) mdaRePsycho!.o -o mdaRePsycho!$(DYN)
mdaRePsycho!.o: $(SRC)mdaRePsycho!.cpp $(SRC)mdaRePsycho!.h
	$(CC) $(CFLAGS) $(SRC)mdaRePsycho!.cpp
#----------------------------------------
mdaRezFilter: $(OBJC) mdaRezFilter.o
	$(CC) $(LFLAGS) $(OBJC) mdaRezFilter.o -o mdaRezFilter$(DYN)
mdaRezFilter.o: $(SRC)mdaRezFilter.cpp $(SRC)mdaRezFilter.h
	$(CC) $(CFLAGS) $(SRC)mdaRezFilter.cpp
#----------------------------------------
mdaRingMod: $(OBJC) mdaRingMod.o
	$(CC) $(LFLAGS) $(OBJC) mdaRingMod.o -o mdaRingMod$(DYN)
mdaRingMod.o: $(SRC)mdaRingMod.cpp $(SRC)mdaRingMod.h
	$(CC) $(CFLAGS) $(SRC)mdaRingMod.cpp
#----------------------------------------
mdaRoundPan: $(OBJC) mdaRoundPan.o
	$(CC) $(LFLAGS) $(OBJC) mdaRoundPan.o -o mdaRoundPan$(DYN)
mdaRoundPan.o: $(SRC)mdaRoundPan.cpp $(SRC)mdaRoundPan.h
	$(CC) $(CFLAGS) $(SRC)mdaRoundPan.cpp
#----------------------------------------
mdaShepard: $(OBJC) mdaShepard.o
	$(CC) $(LFLAGS) $(OBJC) mdaShepard.o -o mdaShepard$(DYN)
mdaShepard.o: $(SRC)mdaShepard.cpp $(SRC)mdaShepard.h
	$(CC) $(CFLAGS) $(SRC)mdaShepard.cpp
#----------------------------------------
mdaSplitter: $(OBJC) mdaSplitter.o
	$(CC) $(LFLAGS) $(OBJC) mdaSplitter.o -o mdaSplitter$(DYN)
mdaSplitter.o: $(SRC)mdaSplitter.cpp $(SRC)mdaSplitter.h
	$(CC) $(CFLAGS) $(SRC)mdaSplitter.cpp
#----------------------------------------
mdaStereo: $(OBJC) mdaStereo.o
	$(CC) $(LFLAGS) $(OBJC) mdaStereo.o -o mdaStereo$(DYN)
mdaStereo.o: $(SRC)mdaStereo.cpp $(SRC)mdaStereo.h
	$(CC) $(CFLAGS) $(SRC)mdaStereo.cpp
#----------------------------------------
mdaSubSynth: $(OBJC) mdaSubSynth.o
	$(CC) $(LFLAGS) $(OBJC) mdaSubSynth.o -o mdaSubSynth$(DYN)
mdaSubSynth.o: $(SRC)mdaSubSynth.cpp $(SRC)mdaSubSynth.h
	$(CC) $(CFLAGS) $(SRC)mdaSubSynth.cpp
#----------------------------------------
mdaTalkBox: $(OBJC) mdaTalkBox.o
	$(CC) $(LFLAGS) $(OBJC) mdaTalkBox.o -o mdaTalkBox$(DYN)
mdaTalkBox.o: $(SRC)mdaTalkBox.cpp $(SRC)mdaTalkBox.h
	$(CC) $(CFLAGS) $(SRC)mdaTalkBox.cpp
#----------------------------------------
mdaTestTone: $(OBJC) mdaTestTone.o
	$(CC) $(LFLAGS) $(OBJC) mdaTestTone.o -o mdaTestTone$(DYN)
mdaTestTone.o: $(SRC)mdaTestTone.cpp $(SRC)mdaTestTone.h
	$(CC) $(CFLAGS) $(SRC)mdaTestTone.cpp
#----------------------------------------
mdaThruZero: $(OBJC) mdaThruZero.o
	$(CC) $(LFLAGS) $(OBJC) mdaThruZero.o -o mdaThruZero$(DYN)
mdaThruZero.o: $(SRC)mdaThruZero.cpp $(SRC)mdaThruZero.h
	$(CC) $(CFLAGS) $(SRC)mdaThruZero.cpp
#----------------------------------------
mdaTracker: $(OBJC) mdaTracker.o
	$(CC) $(LFLAGS) $(OBJC) mdaTracker.o -o mdaTracker$(DYN)
mdaTracker.o: $(SRC)mdaTracker.cpp $(SRC)mdaTracker.h
	$(CC) $(CFLAGS) $(SRC)mdaTracker.cpp
#----------------------------------------
mdaTransient: $(OBJC) mdaTransient.o
	$(CC) $(LFLAGS) $(OBJC) mdaTransient.o -o mdaTransient$(DYN)
mdaTransient.o: $(SRC)mdaTransient.cpp $(SRC)mdaTransient.h
	$(CC) $(CFLAGS) $(SRC)mdaTransient.cpp
#----------------------------------------
mdaVocInput: $(OBJC) mdaVocInput.o
	$(CC) $(LFLAGS) $(OBJC) mdaVocInput.o -o mdaVocInput$(DYN)
mdaVocInput.o: $(SRC)mdaVocInput.cpp $(SRC)mdaVocInput.h
	$(CC) $(CFLAGS) $(SRC)mdaVocInput.cpp
#----------------------------------------
mdaVocoder: $(OBJC) mdaVocoder.o
	$(CC) $(LFLAGS) $(OBJC) mdaVocoder.o -o mdaVocoder$(DYN)
mdaVocoder.o: $(SRC)mdaVocoder.cpp $(SRC)mdaVocoder.h
	$(CC) $(CFLAGS) $(SRC)mdaVocoder.cpp

