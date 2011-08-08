
#-------------------------- ITK/VTK -------------------------
# Provide an option to supply pre-built versions of ITK and VTK.
# If not found, MITK will build them itself during superbuild
set(VTK_DIR "" CACHE PATH "Use the supplied VTK installation/binary dir instead of superbuilding it")
set(ITK_DIR "" CACHE PATH "Use the supplied ITK installation/binary dir instead of superbuilding it")
mark_as_advanced(VTK_DIR ITK_DIR)

#---------------------------- MITK --------------------------

if(MITK_DIR)

  # sanity checks in the case of a pre-built MITK
  set(my_itk_dir ${ITK_DIR})
  set(my_vtk_dir ${VTK_DIR})
  set(my_qmake_executable ${QT_QMAKE_EXECUTABLE})

  find_package(MITK REQUIRED)
  
  if(my_itk_dir AND NOT my_itk_dir STREQUAL ${ITK_DIR})
    message(FATAL_ERROR "ITK packages do not match:\n   ${MY_PROJECT_NAME}: ${my_itk_dir}\n  MITK: ${ITK_DIR}")
  endif()
  
  if(my_vtk_dir AND NOT my_vtk_dir STREQUAL ${VTK_DIR})
    message(FATAL_ERROR "VTK packages do not match:\n   ${MY_PROJECT_NAME}: ${my_vtk_dir}\n  MITK: ${VTK_DIR}")
  endif()
  
  if(my_qmake_executable AND NOT my_qmake_executable STREQUAL ${MITK_QMAKE_EXECUTABLE})
    message(FATAL_ERROR "Qt qmake does not match:\n   ${MY_PROJECT_NAME}: ${my_qmake_executable}\n  MITK: ${MITK_QMAKE_EXECUTABLE}")
  endif()
  
endif()


#-----------------------------------------------------------------------------
# External project settings
#-----------------------------------------------------------------------------

include(ExternalProject)

set(ep_base "${CMAKE_BINARY_DIR}/CMakeExternals")
set_property(DIRECTORY PROPERTY EP_BASE ${ep_base})

set(ep_install_dir "${CMAKE_BINARY_DIR}/CMakeExternals/Install")
set(ep_suffix "-cmake")
set(ep_build_shared_libs ON)
set(ep_build_testing OFF)

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

# Use this value where semi-colons are needed in ep_add args:
set(sep "^^")

##

if(MSVC90 OR MSVC10)
  set(ep_common_C_FLAGS "${CMAKE_C_FLAGS} /bigobj /MP")
  set(ep_common_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP")
else()
  set(ep_common_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX_EXTRA")
  set(ep_common_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLINUX_EXTRA")
endif()

set(ep_common_args 
  -DBUILD_TESTING:BOOL=${ep_build_testing}
  -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
  -DBUILD_SHARED_LIBS:BOOL=${ep_build_shared_libs}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  "-DCMAKE_C_FLAGS:STRING=${ep_common_C_FLAGS}"
  "-DCMAKE_CXX_FLAGS:STRING=${ep_common_CXX_FLAGS}"
)

#-----------------------------------------------------------------------------
# Git protocol option
#-----------------------------------------------------------------------------

option(${MY_PROJECT_NAME}_USE_GIT_PROTOCOL "If behind a firewall turn this OFF to use http instead." OFF)
mark_as_advanced(${MY_PROJECT_NAME}_USE_GIT_PROTOCOL)

set(git_protocol "git")
if(NOT ${MY_PROJECT_NAME}_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

#-----------------------------------------------------------------------------
# ExternalProjects 
#-----------------------------------------------------------------------------

set(external_projects
  MITK
  )

# Include external projects
foreach(p ${external_projects})
  include(CMakeExternals/${p}.cmake)
endforeach()

#-----------------------------------------------------------------------------
# Set superbuild boolean args
#-----------------------------------------------------------------------------

set(my_cmake_boolean_args
  WITH_COVERAGE
  BUILD_TESTING
  ${MY_PROJECT_NAME}_BUILD_ALL_PLUGINS
  )
    
#-----------------------------------------------------------------------------
# Create the final variable containing superbuild boolean args
#-----------------------------------------------------------------------------

set(my_superbuild_boolean_args)
foreach(my_cmake_arg ${my_cmake_boolean_args})
  list(APPEND my_superbuild_boolean_args -D${my_cmake_arg}:BOOL=${${my_cmake_arg}})
endforeach()

#-----------------------------------------------------------------------------
# Project Utilities
#-----------------------------------------------------------------------------

set(proj ${MY_PROJECT_NAME}-Utilities)
ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    # Mandatory dependencies
    ${MITK_DEPENDS}
    # Optional dependencies 
)

#-----------------------------------------------------------------------------
# Project Configure
#-----------------------------------------------------------------------------

set(proj ${MY_PROJECT_NAME}-Configure)

ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${ep_common_args}
    ${my_superbuild_boolean_args}
    -D${MY_PROJECT_NAME}_USE_SUPERBUILD:BOOL=OFF
    -D${MY_PROJECT_NAME}_CONFIGURED_VIA_SUPERBUILD:BOOL=ON
    -DCTEST_USE_LAUNCHERS:BOOL=${CTEST_USE_LAUNCHERS}
    -D${MY_PROJECT_NAME}_SUPERBUILD_BINARY_DIR:PATH=${PROJECT_BINARY_DIR}
    -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
    -DMITK_DIR:PATH=${MITK_DIR}
    -DITK_DIR:PATH=${ITK_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}

  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/${MY_PROJECT_NAME}-build
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${MY_PROJECT_NAME}-Utilities
  )


#-----------------------------------------------------------------------------
# Project
#-----------------------------------------------------------------------------

# The variable SUPERBUILD_EXCLUDE_${MY_PROJECT_NAME}BUILD_TARGET should be set when submitting to a dashboard
if(NOT DEFINED SUPERBUILD_EXCLUDE_${MY_PROJECT_NAME}BUILD_TARGET OR NOT SUPERBUILD_EXCLUDE_${MY_PROJECT_NAME}BUILD_TARGET)
  set(proj ${MY_PROJECT_NAME}-build)
  ExternalProject_Add(${proj}
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ""
    CMAKE_GENERATOR ${gen}
    BUILD_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/${MY_PROJECT_NAME}-build --config ${CMAKE_CFG_INTDIR}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINARY_DIR ${MY_PROJECT_NAME}-build
    INSTALL_COMMAND ""
    DEPENDS
      "${MY_PROJECT_NAME}-Configure"
    )
endif()

#-----------------------------------------------------------------------------
# Custom target allowing to drive the build of the project itself
#-----------------------------------------------------------------------------

add_custom_target(${MY_PROJECT_NAME}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/${MY_PROJECT_NAME}-build --config ${CMAKE_CFG_INTDIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${MY_PROJECT_NAME}-build
)

