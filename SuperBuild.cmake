
#-----------------------------------------------------------------------------
# Convenient macro allowing to download a file
#-----------------------------------------------------------------------------

MACRO(downloadFile url dest)
  FILE(DOWNLOAD ${url} ${dest} STATUS status)
  LIST(GET status 0 error_code)
  LIST(GET status 1 error_msg)
  IF(error_code)
    MESSAGE(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
  ENDIF()
ENDMACRO()

#-----------------------------------------------------------------------------
# MITK Prerequisites
#-----------------------------------------------------------------------------

if(UNIX AND NOT APPLE)
  #----------------------------- libxt-dev --------------------
  INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)

  set(CMAKE_REQUIRED_INCLUDES "/usr/include/X11/")
  CHECK_INCLUDE_FILE("StringDefs.h" STRING_DEFS_H)
  if(NOT STRING_DEFS_H)
    message(FATAL_ERROR "error: could not find StringDefs.h provided by libxt-dev")
  endif()

  set(CMAKE_REQUIRED_INCLUDES "/usr/include/")
  CHECK_INCLUDE_FILE("tiff.h" TIFF_H)
  if(NOT TIFF_H)
    message(FATAL_ERROR "error: could not find tiff.h - libtiff4-dev needs to be installed")
  endif()

  CHECK_INCLUDE_FILE("tcpd.h" LIB_WRAP)
  if(NOT LIB_WRAP)
    message(FATAL_ERROR "error: could not find tcpd.h - libwrap0-dev needs to be installed")
  endif()

endif()


#-----------------------------------------------------------------------------
# ExternalProjects
#-----------------------------------------------------------------------------

SET(external_projects
  VTK
  GDCM
  CableSwig
  ITK
  Boost
  DCMTK
  CTK
  OpenCV
  MITKData
  )


set(MITK_USE_CableSwig ${MITK_USE_Python})
set(MITK_USE_GDCM 1)
set(MITK_USE_ITK 1)
set(MITK_USE_VTK 1)
  
foreach(proj VTK GDCM CableSwig ITK DCMTK CTK OpenCV)
  if(MITK_USE_${proj})
    set(EXTERNAL_${proj}_DIR "${${proj}_DIR}" CACHE PATH "Path to ${proj} build directory")
    mark_as_advanced(EXTERNAL_${proj}_DIR)
    if(EXTERNAL_${proj}_DIR)
      set(${proj}_DIR ${EXTERNAL_${proj}_DIR})
    endif()
  endif()
endforeach()

if(MITK_USE_Boost)
  set(EXTERNAL_BOOST_ROOT "${BOOST_ROOT}" CACHE PATH "Path to Boost directory")
  mark_as_advanced(EXTERNAL_BOOST_ROOT)
  if(EXTERNAL_BOOST_ROOT)
    set(BOOST_ROOT ${EXTERNAL_BOOST_ROOT})
  endif()
endif()

if(BUILD_TESTING)
  set(EXTERNAL_MITK_DATA_DIR "${MITK_DATA_DIR}" CACHE PATH "Path to the MITK data directory")
  mark_as_advanced(EXTERNAL_MITK_DATA_DIR)
  if(EXTERNAL_MITK_DATA_DIR)
    set(MITK_DATA_DIR ${EXTERNAL_MITK_DATA_DIR})
  endif()
endif()

# Look for git early on, if needed
if((BUILD_TESTING AND NOT EXTERNAL_MITK_DATA_DIR) OR
   (MITK_USE_CTK AND NOT EXTERNAL_CTK_DIR))
  find_package(Git REQUIRED)
endif()

#-----------------------------------------------------------------------------
# External project settings
#-----------------------------------------------------------------------------

INCLUDE(ExternalProject)

SET(ep_base "${CMAKE_BINARY_DIR}/CMakeExternals")
SET_PROPERTY(DIRECTORY PROPERTY EP_BASE ${ep_base})

SET(ep_install_dir ${ep_base}/Install)
#SET(ep_build_dir ${ep_base}/Build)
SET(ep_source_dir ${ep_base}/Source)
#SET(ep_parallelism_level)
SET(ep_build_shared_libs ON)
SET(ep_build_testing OFF)

# Compute -G arg for configuring external projects with the same CMake generator:
IF(CMAKE_EXTRA_GENERATOR)
  SET(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
ELSE()
  SET(gen "${CMAKE_GENERATOR}")
ENDIF()

# Use this value where semi-colons are needed in ep_add args:
set(sep "^^")

##

IF(MSVC90 OR MSVC10)
  SET(ep_common_C_FLAGS "${CMAKE_C_FLAGS} /bigobj /MP")
  SET(ep_common_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP")
ELSE()
  SET(ep_common_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX_EXTRA")
  SET(ep_common_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLINUX_EXTRA")
ENDIF()

SET(ep_common_args
  -DBUILD_TESTING:BOOL=${ep_build_testing}
  -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
  -DBUILD_SHARED_LIBS:BOOL=ON
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_FLAGS:STRING=${ep_common_C_FLAGS}
  -DCMAKE_CXX_FLAGS:STRING=${ep_common_CXX_FLAGS}
  #debug flags
  -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
  -DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG}
  #release flags
  -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
  -DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE}
  #relwithdebinfo
  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
  -DCMAKE_C_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_C_FLAGS_RELWITHDEBINFO}
)

# Include external projects
FOREACH(p ${external_projects})
  INCLUDE(CMakeExternals/${p}.cmake)
ENDFOREACH()

#-----------------------------------------------------------------------------
# Set superbuild boolean args
#-----------------------------------------------------------------------------

SET(mitk_cmake_boolean_args
  BUILD_SHARED_LIBS
  WITH_COVERAGE
  BUILD_TESTING

  MITK_USE_QT
  MITK_BUILD_ALL_PLUGINS
  MITK_BUILD_ALL_APPS
  MITK_BUILD_TUTORIAL
  MITK_USE_Boost
  MITK_USE_SYSTEM_Boost
  MITK_USE_BLUEBERRY
  MITK_USE_CTK
  MITK_USE_DCMTK
  MITK_USE_OpenCV
  MITK_USE_Python
  )

#-----------------------------------------------------------------------------
# Create the final variable containing superbuild boolean args
#-----------------------------------------------------------------------------

SET(mitk_superbuild_boolean_args)
FOREACH(mitk_cmake_arg ${mitk_cmake_boolean_args})
  LIST(APPEND mitk_superbuild_boolean_args -D${mitk_cmake_arg}:BOOL=${${mitk_cmake_arg}})
ENDFOREACH()

IF(MITK_BUILD_ALL_PLUGINS)
  LIST(APPEND mitk_superbuild_boolean_args -DBLUEBERRY_BUILD_ALL_PLUGINS:BOOL=ON)
ENDIF()

#-----------------------------------------------------------------------------
# MITK Utilities
#-----------------------------------------------------------------------------

set(proj MITK-Utilities)
ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    # Mandatory dependencies
    ${VTK_DEPENDS}
    ${ITK_DEPENDS}
    # Optionnal dependencies
    ${Boost_DEPENDS}
    ${CTK_DEPENDS}
    ${DCMTK_DEPENDS}
    ${OpenCV_DEPENDS}
    ${MITK-Data_DEPENDS}
)

#-----------------------------------------------------------------------------
# MITK Configure
#-----------------------------------------------------------------------------

IF(MITK_INITIAL_CACHE_FILE)
  SET(mitk_initial_cache_arg -C "${MITK_INITIAL_CACHE_FILE}")
ENDIF()

SET(mitk_optional_cache_args )
FOREACH(type RUNTIME ARCHIVE LIBRARY)
  IF(DEFINED CTK_PLUGIN_${type}_OUTPUT_DIRECTORY)
    LIST(APPEND mitk_optional_cache_args -DCTK_PLUGIN_${type}_OUTPUT_DIRECTORY:PATH=${CTK_PLUGIN_${type}_OUTPUT_DIRECTORY})
  ENDIF()
ENDFOREACH()

SET(proj MITK-Configure)

ExternalProject_Add(${proj}
  LIST_SEPARATOR ^^
  DOWNLOAD_COMMAND ""
  CMAKE_GENERATOR ${gen}
  CMAKE_CACHE_ARGS
    ${ep_common_args}
    ${mitk_superbuild_boolean_args}
    ${mitk_optional_cache_args}
    -DMITK_USE_SUPERBUILD:BOOL=OFF
    -DMITK_CMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${MITK_CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    -DMITK_CMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${MITK_CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    -DMITK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${MITK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    -DCTEST_USE_LAUNCHERS:BOOL=${CTEST_USE_LAUNCHERS}
    -DMITK_CTEST_SCRIPT_MODE:STRING=${MITK_CTEST_SCRIPT_MODE}
    -DMITK_SUPERBUILD_BINARY_DIR:PATH=${MITK_BINARY_DIR}
    -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
    -DMITK_KWSTYLE_EXECUTABLE:FILEPATH=${MITK_KWSTYLE_EXECUTABLE}
    -DMITK_MODULES_TO_BUILD:INTERNAL=${MITK_MODULES_TO_BUILD}
    -DCTK_DIR:PATH=${CTK_DIR}
    -DDCMTK_DIR:PATH=${DCMTK_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}     # FindVTK expects VTK_DIR
    -DITK_DIR:PATH=${ITK_DIR}     # FindITK expects ITK_DIR
    -DOpenCV_DIR:PATH=${OpenCV_DIR}
    -DGDCM_DIR:PATH=${GDCM_DIR}
    -DBOOST_ROOT:PATH=${BOOST_ROOT}
    -DMITK_USE_Boost_LIBRARIES:STRING=${MITK_USE_Boost_LIBRARIES}
    -DMITK_DATA_DIR:PATH=${MITK_DATA_DIR}
    -DMITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES:STRING=${MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES}
    -DMITK_ACCESSBYITK_FLOATING_PIXEL_TYPES:STRING=${MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES}
    -DMITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES:STRING=${MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES}
    -DMITK_ACCESSBYITK_DIMENSIONS:STRING=${MITK_ACCESSBYITK_DIMENSIONS}
  CMAKE_ARGS
    ${mitk_initial_cache_arg}

  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/MITK-build
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    MITK-Utilities
  )


#-----------------------------------------------------------------------------
# MITK
#-----------------------------------------------------------------------------

IF(CMAKE_GENERATOR MATCHES ".*Makefiles.*")
  SET(mitk_build_cmd "$(MAKE)")
ELSE()
  SET(mitk_build_cmd ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/MITK-build --config ${CMAKE_CFG_INTDIR})
ENDIF()

IF(NOT DEFINED SUPERBUILD_EXCLUDE_MITKBUILD_TARGET OR NOT SUPERBUILD_EXCLUDE_MITKBUILD_TARGET)
  SET(MITKBUILD_TARGET_ALL_OPTION "ALL")
ELSE()
  SET(MITKBUILD_TARGET_ALL_OPTION "")
ENDIF()

ADD_CUSTOM_TARGET(MITK-build ${MITKBUILD_TARGET_ALL_OPTION}
  COMMAND ${mitk_build_cmd}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/MITK-build
  DEPENDS MITK-Configure
  )

#-----------------------------------------------------------------------------
# Custom target allowing to drive the build of the MITK project itself
#-----------------------------------------------------------------------------

ADD_CUSTOM_TARGET(MITK
  COMMAND ${mitk_build_cmd}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/MITK-build
)

