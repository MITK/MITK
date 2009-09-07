FIND_PACKAGE(ITK)
INCLUDE_DIRECTORIES(. .. )

ADD_DEFINITIONS(-DMITK_IPPIC_COMPILE)

SET(IPPIC_FILES
  ipPicInfo.c ipPicType.c                           
  ipPicAddT.c ipPicQueryT.c ipPicDelT.c ipPicGetT.c
  ipPicOldGet.c ipPicOldGetH.c ipPicOldGetS.c       
  ipPicGet.c ipPicGetH.c ipPicGetS.c                
  ipPicPut.c ipPicPutS.c                            
  ipPicGetMem.c ipPicPutMem.c                       
  ipPicCopyS.c ipPicCopyH.c                         
  ipPicNew.c ipPicClear.c ipPicFree.c ipPicClone.c  
  ipEndian.c ipFRead.c ipFWrite.c                   
  ipError.c                                         
  ipPicSize.c ipPicElements.c                       
  ipPicTSVSize.c ipPicTSVElements.c
  )
ADD_LIBRARY(ipPic ${MITK_WIN32_FORCE_STATIC} ${IPPIC_FILES})
IF(USE_ITKZLIB)
  TARGET_LINK_LIBRARIES(ipPic itkzlib)
ELSE(USE_ITKZLIB)
  TARGET_LINK_LIBRARIES(ipPic z)
ENDIF(USE_ITKZLIB)

MITK_CREATE_MODULE_CONF(ipPic
  INCLUDE_DIRS ..
)

  

# SET(IPPIC_INCLUDE_DIRS ${IP_ROOT}/DataStructures)
# IF(WIN32)
#  SET(IPPIC_INCLUDE_DIRS ${IPPIC_INCLUDE_DIRS} ${IP_ROOT}/DataStructures/ipPic/itkzlib)
# ENDIF(WIN32)
