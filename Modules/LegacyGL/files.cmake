#We are deprecated. Please don't use us.

file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkGLMapper.cpp
  mitkVtkGLMapperWrapper.cpp
  vtkGLMapperProp.cpp
)
