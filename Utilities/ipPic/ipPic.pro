TEMPLATE = lib
TARGET = libipPic

CONFIG	+= warn_on release
unix:CONFIG += staticlib

OBJECTS_DIR = obj
DESTDIR = .

SOURCES = \
      ipPicInfo.c ipPicType.c                           \
      ipPicAddT.c ipPicQueryT.c ipPicDelT.c ipPicGetT.c \
      ipPicOldGet.c ipPicOldGetH.c ipPicOldGetS.c       \
      ipPicGet.c ipPicGetH.c ipPicGetS.c                \
      ipPicPut.c ipPicPutS.c                            \
      ipPicGetMem.c ipPicPutMem.c                       \
      ipPicCopyS.c ipPicCopyH.c                         \
      ipPicNew.c ipPicClear.c ipPicFree.c ipPicClone.c  \
      ipEndian.c ipFRead.c ipFWrite.c                   \
      ipError.c                                         \
      ipPicSize.c ipPicElements.c                       \
      ipPicTSVSize.c ipPicTSVElements.c

win32:SOURCES+= win32/zlib.lib

HEADERS = ipPic.h ipTypes.h ipPicOldP.h ipPicAnnotation.h 

INCLUDEPATH = .. .
win32:INCLUDEPATH+=win32

