#! \brief Get diverse visual studio ids not directly provided by CMake
#!
#! Sets the following variables in the parent scope
#!   VISUAL_STUDIO_VERSION_MAJOR - The Visual Studio Version
#!   VISUAL_STUDIO_PRODUCT_NAME - The Visual Studio Product Name

function(mitkFunctionGetMSVCVersion )

if(MSVC)
  if(MSVC_VERSION GREATER_EQUAL 1910 AND MSVC_VERSION LESS 2000)
    set(VISUAL_STUDIO_PRODUCT_NAME "Visual Studio 2017" PARENT_SCOPE)
    set(VISUAL_STUDIO_VERSION_MAJOR 14 PARENT_SCOPE)
    string(SUBSTRING ${MSVC_VERSION} 3 -1 version_minor)
    set(VISUAL_STUDIO_VERSION_MINOR ${version_minor} PARENT_SCOPE)
  else()
    message(WARNING "Unknown Visual Studio version ${MSVC_VERSION} (CMake/mitkFunctionGetMSVCVersion.cmake)")
  endif()

  if("${CMAKE_GENERATOR}" MATCHES ".*Win64")
    set(CMAKE_LIBRARY_ARCHITECTURE x64 PARENT_SCOPE)
  else()
    set(CMAKE_LIBRARY_ARCHITECTURE x86 PARENT_SCOPE)
  endif()
endif()

endfunction()
