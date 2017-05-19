
set(Caffe2_DIR ${MITK_EXTERNAL_PROJECT_PREFIX})
set(Caffe2_INCLUDE_DIR ${Caffe2_DIR}/include)
set(Caffe2_CMAKE_DEBUG_POSTFIX d)

set(Caffe2_LIBRARIES)
  
FOREACH(POSTFIX _CPU;_GPU)

# Find all libraries, store debug and release separately
  # Find Release libraries
  find_library(Caffe2_LIBRARY_RELEASE
    Caffe2${POSTFIX}
    PATHS
    ${Caffe2_DIR}/lib
    ${Caffe2_DIR}/lib/Release
    ${Caffe2_DIR}/lib/RelWithDebInfo
    ${Caffe2_DIR}/bin
    ${Caffe2_DIR}/bin/Release
    ${Caffe2_DIR}/bin/RelWithDebInfo
    NO_DEFAULT_PATH
    )

  # Find Debug libraries
  find_library(Caffe2_LIBRARY_DEBUG
    Caffe2${POSTFIX}${Caffe2_CMAKE_DEBUG_POSTFIX}
    PATHS
    ${Caffe2_DIR}/lib
    ${Caffe2_DIR}/lib/Debug
    ${Caffe2_DIR}/bin
    ${Caffe2_DIR}/bin/Debug
    NO_DEFAULT_PATH
    )

  # Add libraries to variable according to build type

  if(Caffe2_LIBRARY_RELEASE)
    list(APPEND Caffe2_LIBRARIES optimized ${Caffe2_LIBRARY_RELEASE})
    list(APPEND Caffe2_LIBRARIES optimized /usr/lib/x86_64-linux-gnu/libprotobuf.so)
  endif()

  if(Caffe2_LIBRARY_DEBUG)
    list(APPEND Caffe2_LIBRARIES debug ${Caffe2_LIBRARY_DEBUG})
    list(APPEND Caffe2_LIBRARIES debug /usr/lib/x86_64-linux-gnu/libprotobuf.so)
  endif()
  
ENDFOREACH()

find_package_handle_standard_args(Caffe2
  FOUND_VAR Caffe2_FOUND
  REQUIRED_VARS Caffe2_INCLUDE_DIR Caffe2_LIBRARIES
)
