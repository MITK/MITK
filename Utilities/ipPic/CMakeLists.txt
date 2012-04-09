find_package(ITK)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DMITK_IPPIC_COMPILE)

set(IPPIC_FILES
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

add_library(mitkIpPic ${MITK_WIN32_FORCE_STATIC} ${IPPIC_FILES})
if(USE_ITKZLIB)
  target_link_libraries(mitkIpPic itkzlib)
else(USE_ITKZLIB)
  target_link_libraries(mitkIpPic z)
endif(USE_ITKZLIB)

MITK_CREATE_MODULE_CONF(mitkIpPic
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..
)

