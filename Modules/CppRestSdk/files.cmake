file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkRESTClientMicroService.cpp
  mitkRESTServerMicroService.cpp
  mitkCppRestSdkActivator.cpp
  mitkIRESTManager.cpp
  mitkRESTManager.cpp
  mitkIRESTObserver.cpp
  mitkCppRestSdk.cpp
)
set(MOC_H_FILES
include/mitkRESTServerMicroService.h
include/mitkRESTManager.h)