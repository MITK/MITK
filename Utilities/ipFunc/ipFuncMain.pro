TEMPLATE	= app
CONFIG		= console warn_on release
HEADERS		= mitkIpFunc.h 

SOURCES		= ipFunc.c 

unix:LIBS              = -L../../DataStructures/ipPic 

unix:LIBS       += -lz  -L. -lipFunc -L../../DatasStructures/ipPic -lipPic

win32:LIBS	+= $(IPDIR)/piano/ipPic/lib/libipPic.lib $(IPDIR)/piano/ipFunc/lib/libipFunc.lib 

INCLUDEPATH = ../../DataStructures ../../DataStructures/ipPic

TARGET = ipFunc


#DESTDIR = lib

#INTERFACES	= 

win32:QMAKE_CXXFLAGS+=-GX #exception handling
win32:QMAKE_CXXFLAGS_DEBUG+=-ZI #edit and continue
win32:QMAKE_LFLAGS_DEBUG+=/nodefaultlib:"libcd" /nodefaultlib:"MSVCRT"
win32:QMAKE_LFLAGS_DEBUG-=/incremental:no
win32:QMAKE_LFLAGS_DEBUG+=/incremental:yes

