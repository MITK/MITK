set(IPSEGMENTATION_FILES
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

include_directories(.. ../ann/include ${MITK_INCLUDE_DIRS} ${ANN_INCLUDE_DIR} ../ipPic)
add_library(ipSegmentation${LIBPOSTFIX} ${MITK_WIN32_FORCE_STATIC} ${IPSEGMENTATION_FILES})
target_link_libraries(ipSegmentation ann mitkIpPic)

MITK_CREATE_MODULE_CONF(ipSegmentation
  DEPENDS mitkIpPic ann
)

