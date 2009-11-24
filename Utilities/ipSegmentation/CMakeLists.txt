SET(IPSEGMENTATION_FILES
    ipSegmentationError.c
    ipSegmentationNew.c
    ipSegmentationFree.c
    ipSegmentationClear.c
    ipSegmentationCombineRegion.c
    ipSegmentationRegionGrower.cpp
    ipSegmentationContourExtraction.cpp
    ipSegmentationReplaceRegion.cpp
    ipSegmentationRegionCutter.cpp
    ipSegmentationGrowerHistory.cpp
    ipSegmentationContourUtils.cpp
    ipSegmentationInterpolate.c
    ipSegmentationUndo.c
)

INCLUDE_DIRECTORIES(.. ../ann/include ${MITK_INCLUDE_DIRS} ${ANN_INCLUDE_DIR} ../ipPic)
ADD_LIBRARY(ipSegmentation${LIBPOSTFIX} ${MITK_WIN32_FORCE_STATIC} ${IPSEGMENTATION_FILES})
TARGET_LINK_LIBRARIES(ipSegmentation ann mitkIpPic)

MITK_CREATE_MODULE_CONF(ipSegmentation
  DEPENDS mitkIpPic ann
)

IF(NOT MITK_WIN32_FORCE_STATIC)
  MITK_INSTALL_TARGETS(ipSegmentation)
ENDIF()
