
set(Caffe2_DIR ${MITK_EXTERNAL_PROJECT_PREFIX})
set(Caffe2_INCLUDE_DIR ${Caffe2_DIR}/include)
set(Caffe2_CMAKE_DEBUG_POSTFIX d)
set(Caffe2_LIBRARIES)

set(CUDA_DIR $ENV{CUDA_DIR})
if (NOT CUDA_DIR)
  find_path(CUDA_INSTALL_DIR cuda)
  set(CUDA_DIR ${CUDA_INSTALL_DIR}/cuda)
endif()

if (CUDA_DIR)
  find_library (CUDART_LIB cudart PATHS ${CUDA_DIR}/lib64)
  find_library (CUDNN_LIB cudnn PATHS ${CUDA_DIR}/lib64)
  find_library (CUBLAS_LIB cublas PATHS ${CUDA_DIR}/lib64)
  find_library (CURAND_LIB curand PATHS ${CUDA_DIR}/lib64)
  find_library (CUDA_LIB cuda)
endif()
find_library (PROTOBUF_LIB protobuf)

set(NCCL_LIB_DIR ${Caffe2_DIR}/src/Caffe2/third_party/nccl/build/lib/)

FOREACH(POSTFIX _CPU;_GPU)

# Find all libraries, store debug and release separately
  # Find Release libraries
  find_library(Caffe2_LIBRARY_RELEASE${POSTFIX}
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
  find_library(Caffe2_LIBRARY_DEBUG${POSTFIX}
    Caffe2${POSTFIX}${Caffe2_CMAKE_DEBUG_POSTFIX}
    PATHS
    ${Caffe2_DIR}/lib
    ${Caffe2_DIR}/lib/Debug
    ${Caffe2_DIR}/bin
    ${Caffe2_DIR}/bin/Debug
    NO_DEFAULT_PATH
    )
      
  # Add libraries to variable according to build type

  if(Caffe2_LIBRARY_RELEASE${POSTFIX})
    list(APPEND Caffe2_LIBRARIES optimized ${Caffe2_LIBRARY_RELEASE${POSTFIX}})
    list(APPEND Caffe2_LIBRARIES optimized ${PROTOBUF_LIB})
    
    if(${POSTFIX} MATCHES "_GPU")
      find_library(NCCL_LIB_RELEASE nccl PATHS ${NCCL_LIB_DIR})
      list(APPEND Caffe2_LIBRARIES optimized ${NCCL_LIB_RELEASE})
      list(APPEND Caffe2_LIBRARIES optimized ${CUDART_LIB})
      list(APPEND Caffe2_LIBRARIES optimized ${CUDNN_LIB})
      list(APPEND Caffe2_LIBRARIES optimized ${CUBLAS_LIB})
      list(APPEND Caffe2_LIBRARIES optimized ${CURAND_LIB})
      list(APPEND Caffe2_LIBRARIES optimized ${CUDA_LIB})
    endif()
  
  endif()

  if(Caffe2_LIBRARY_DEBUG${POSTFIX})
    list(APPEND Caffe2_LIBRARIES debug ${Caffe2_LIBRARY_DEBUG${POSTFIX}})
    list(APPEND Caffe2_LIBRARIES debug ${PROTOBUF_LIB})
    
    if(${POSTFIX} MATCHES "_GPU")
      find_library(NCCL_LIB_DEBUG nccl PATHS ${NCCL_LIB_DIR})
      list(APPEND Caffe2_LIBRARIES debug ${NCCL_LIB_DEBUG})
      list(APPEND Caffe2_LIBRARIES debug ${CUDART_LIB})
      list(APPEND Caffe2_LIBRARIES debug ${CUDNN_LIB})
      list(APPEND Caffe2_LIBRARIES debug ${CUBLAS_LIB})
      list(APPEND Caffe2_LIBRARIES debug ${CURAND_LIB})
      list(APPEND Caffe2_LIBRARIES debug ${CUDA_LIB})
    endif()
    
  endif()
  
ENDFOREACH()

find_package_handle_standard_args(Caffe2
  FOUND_VAR Caffe2_FOUND
  REQUIRED_VARS Caffe2_INCLUDE_DIR Caffe2_LIBRARIES PROTOBUF_LIB CUDA_DIR
)
