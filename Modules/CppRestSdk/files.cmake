file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkRESTServer.cpp
  mitkRESTClient.cpp
  mitkDICOMWeb.cpp
  mitkRESTClientMicroService.cpp
  mitkRESTServerMicroService.cpp
  mitkCppRestSdkActivator.cpp
  mitkIRESTClient.cpp
  mitkIRESTServer.cpp
  mitkIRESTManager.cpp
  mitkRESTManager.cpp
)
