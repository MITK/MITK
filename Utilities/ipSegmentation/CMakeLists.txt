
#include_directories(.. ../ann/include ${MITK_INCLUDE_DIRS} ${ANN_INCLUDE_DIR} ../ipPic)
#add_library(ipSegmentation${LIBPOSTFIX} ${MITK_WIN32_FORCE_STATIC} ${IPSEGMENTATION_FILES}#)
#target_link_libraries(ipSegmentation ann mitkIpPic)

set(module_options NO_INIT GCC_DEFAULT_VISIBILITY)
if(MITK_WIN32_FORCE_STATIC)
  list(APPEND module_options FORCE_STATIC)
endif()

MITK_CREATE_MODULE(ipSegmentation
  DEPENDS mitkIpPic
  PACKAGE_DEPENDS ANN
  ${module_options}
)

