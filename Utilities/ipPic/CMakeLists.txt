FIND_PACKAGE(ITK)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

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

ADD_LIBRARY(mitkIpPic ${MITK_WIN32_FORCE_STATIC} ${IPPIC_FILES})
IF(USE_ITKZLIB)
  TARGET_LINK_LIBRARIES(mitkIpPic itkzlib)
ELSE(USE_ITKZLIB)
  TARGET_LINK_LIBRARIES(mitkIpPic z)
ENDIF(USE_ITKZLIB)

MITK_CREATE_MODULE_CONF(mitkIpPic
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..
)

