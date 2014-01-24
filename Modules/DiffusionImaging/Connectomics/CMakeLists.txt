# Boost throws a lot of unused variable warnings on linux

if(UNIX)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=unused-variable" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

MITK_CREATE_MODULE( Connectomics
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms IODataStructures Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS DiffusionCore FiberTracking
  PACKAGE_DEPENDS VTK|vtkInfovisLayout
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
