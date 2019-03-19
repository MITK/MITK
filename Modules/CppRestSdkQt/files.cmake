file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
mitkRESTManagerQt.cpp
mitkCppRestSdkQtActivator.cpp
mitkRESTServerMicroServiceQt.cpp
)

set(MOC_H_FILES
include/mitkRESTManagerQt.h
include/mitkRESTServerMicroServiceQt.h)