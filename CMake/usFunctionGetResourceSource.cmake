#! \ingroup MicroServicesCMake
#! \brief Get a source file name for handling resource dependencies
#!
#! This CMake function retrieves the name of a generated file which has to be added
#! to a modules source file list to set-up resource file dependencies. This ensures
#! that changed resource files will automatically be re-added to the module.
#!
#! Example usage:
#! \code{.cmake}
#! set(module_srcs mylib.cpp)
#! usFunctionGetResourceSource(TARGET mylib
#!                             OUT module_srcs
#!                            )
#! add_library(mylib ${module_srcs})
#! \endcode
#!
#! \param TARGET (required) The name of the target to which the resource files are added.
#! \param OUT (required) A list variable to which the file name will be appended.
#!
#! \sa usFunctionAddResources
#! \sa usFunctionEmbedResources
#!
function(usFunctionGetResourceSource)
  cmake_parse_arguments(_res "" "TARGET;OUT" "" ${ARGN})
  if(NOT _res_TARGET)
    message(SEND_ERROR "TARGET must not be empty")
  endif()
  if(NOT _res_OUT)
    message(SEND_ERROR "OUT argument must not be empty")
  endif()

  set(_out "${CMAKE_CURRENT_BINARY_DIR}/us_${_res_TARGET}/us_resources${US_RESOURCE_SOURCE_SUFFIX}")

  set(${_res_OUT} ${${_res_OUT}} ${_out} PARENT_SCOPE)
endfunction()
