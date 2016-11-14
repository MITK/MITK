A C++/CLI Wrapper for Ophir Pyroelectrical Sensors

compile release version of wapper via: cl /clr /LD OphirPyroWrapper.cpp
compile debug version of wapper via: cl /clr /LDd OphirPyroWrapper.cpp
you'll need: Interop.OphirLMMeasurementLib.dll

compile shitty console example via: cl ConsoleExample.cpp OphirPyroWrapper.lib

tested with:
VS2015, Windows 10 + 8, Starlab 3.20

todo:
- write documentation

thanks to:
pragmateek for http://pragmateek.com/using-c-from-native-c-with-the-help-of-ccli-v2/