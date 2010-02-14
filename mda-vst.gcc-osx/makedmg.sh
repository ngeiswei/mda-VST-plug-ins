#!/bin/sh

DMG="mda-vst-bin-osx-UB.dmg"

hdiutil create -volname mda-vst tmp-mda-vst.dmg -fs HFS+J -size 16m
hdiutil attach -noverify "tmp-mda-vst.dmg"
cp -R ./*.vst "/Volumes/mda-vst"
hdiutil detach "/Volumes/mda-vst"
hdiutil convert "tmp-mda-vst.dmg" -format UDZO -o $DMG
rm -f "tmp-mda-vst.dmg"



