TEMPLATE = lib
TARGET = libipPic

CONFIG	+= warn_on release
unix:CONFIG += staticlib

OBJECTS_DIR = obj
DESTDIR = .

SOURCES = \
      mitkIpPicInfo.c mitkIpPicType.c                           \
      mitkIpPicAddT.c mitkIpPicQueryT.c mitkIpPicDelT.c mitkIpPicGetT.c \
      mitkIpPicOldGet.c mitkIpPicOldGetH.c mitkIpPicOldGetS.c       \
      mitkIpPicGet.c mitkIpPicGetH.c mitkIpPicGetS.c                \
      mitkIpPicPut.c mitkIpPicPutS.c                            \
      mitkIpPicGetMem.c mitkIpPicPutMem.c                       \
      mitkIpPicCopyS.c mitkIpPicCopyH.c                         \
      mitkIpPicNew.c mitkIpPicClear.c mitkIpPicFree.c mitkIpPicClone.c  \
      ipEndian.c ipFRead.c ipFWrite.c                   \
      ipError.c                                         \
      mitkIpPicSize.c mitkIpPicElements.c                       \
      mitkIpPicTSVSize.c mitkIpPicTSVElements.c

win32:SOURCES+= win32/zlib.lib

HEADERS = mitkIpPic.h mitkIpTypes.h mitkIpPicOldP.h mitkIpPicAnnotation.h 

INCLUDEPATH = .. .
win32:INCLUDEPATH+=win32

