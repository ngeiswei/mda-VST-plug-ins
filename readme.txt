mda VST plug-ins

This archive contains the source code for the mda freeware VST 
plug-ins, with the least restrictive licensing I could find.

Projects are provided for Visual C++ 5, Visual C++ 8, and XCode 2.
You will need a copy of the VST 2.4 SDK to compile the plug-ins,
unless you are only interested in the AU versions (kindly ported 
by Sophia Poirier).  Code for "mda SpecMeter" is included but does
not compile with any recent version of VSTGUI.  The GUIs for
"mda Piano" and "mda ePiano" are not included for the same reason,
and also because nobody really liked them.

This code is definitely not an example of how to write plug-ins!
It's obvious that I didn't know much C++ when I started, and some
of the optimizations might have worked on a 486 processor but are
not relevant today.  The code is very raw with no niceties like
parameter de-zipping, but maybe you'll find some useful stuff in
there.  Hopefully someone will feel like tidying everything up and
put the code in a more generic format that can be wrapped into any
current or future plug-in format.

Paul Kellett (paul.kellett@mda-vst.com), June 2008
