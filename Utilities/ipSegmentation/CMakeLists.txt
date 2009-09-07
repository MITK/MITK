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
TARGET_LINK_LIBRARIES(ipSegmentation ann ipPic)

MITK_CREATE_MODULE_CONF(ipSegmentation
  DEPENDS ipPic ann
)
