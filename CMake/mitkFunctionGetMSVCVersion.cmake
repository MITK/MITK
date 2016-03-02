#! \brief Get diverse visual studio ids not directly provided by CMake
#!
#! Sets the following variables in the parent scope
#!   VISUAL_STUDIO_VERSION_MAJOR - The Visual Studio Version
#!   VISUAL_STUDIO_PRODUCT_NAME - The Visual Studio Product Name

function(mitkFunctionGetMSVCVersion )

if(MSVC)
  if(MSVC_VERSION EQUAL 1600)
    set(VISUAL_STUDIO_VERSION_MAJOR "10" PARENT_SCOPE)
    set(VISUAL_STUDIO_PRODUCT_NAME "Visual Studio 2010" PARENT_SCOPE)
  elseif(MSVC_VERSION EQUAL 1700)
    set(VISUAL_STUDIO_VERSION_MAJOR "11" PARENT_SCOPE)
    set(VISUAL_STUDIO_PRODUCT_NAME "Visual Studio 2012" PARENT_SCOPE)
  elseif(MSVC_VERSION EQUAL 1800)
    set(VISUAL_STUDIO_VERSION_MAJOR "12" PARENT_SCOPE)
    set(VISUAL_STUDIO_PRODUCT_NAME "Visual Studio 2013" PARENT_SCOPE)
  elseif(MSVC_VERSION EQUAL 1900)
    set(VISUAL_STUDIO_VERSION_MAJOR "14" PARENT_SCOPE)
    set(VISUAL_STUDIO_PRODUCT_NAME "Visual Studio 2015" PARENT_SCOPE)
  else()
    message(WARNING "Unknown Visual Studio version, some functionality might not be available. Please update mitkFunctionGetMSVCVersion.cmake.")
  endif()

  if("${CMAKE_GENERATOR}" MATCHES ".*Win64")
    set(CMAKE_LIBRARY_ARCHITECTURE x64 PARENT_SCOPE)
  else()
    set(CMAKE_LIBRARY_ARCHITECTURE x86 PARENT_SCOPE)
  endif()
endif()

endfunction()
