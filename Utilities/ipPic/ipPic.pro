TEMPLATE = lib
TARGET = ipPic
CONFIG += warn_on debug

OBJECTS_DIR = obj

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

HEADERS = ipPic.h ipTypes.h ipPicOldP.h ipPicAnnotation.h 

INCLUDEPATH = .. .

