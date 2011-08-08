#-----------------------------------------------------------------------------
# MITK
#-----------------------------------------------------------------------------

set(MITK_DEPENDS)
set(proj_DEPENDENCIES)
set(proj MITK)

if(NOT MITK_DIR)

  #-----------------------------------------------------------------------------
  # Generate cmake variable names for BlueBerry bundles
  #-----------------------------------------------------------------------------

  option(MITK_USE_SUPERBUILD "Use superbuild for MITK" ON)
  option(MITK_USE_BLUEBERRY "Build the BlueBerry platform in MITK" ON)
  option(MITK_BUILD_TUTORIAL "Build the MITK tutorial" OFF)
  option(MITK_BUILD_ALL_PLUGINS "Build all MITK plugins" OFF)
  option(MITK_USE_CTK "Use CTK in MITK" ${MITK_USE_BLUEBERRY})
  option(MITK_USE_GDCMIO "Use the GDCMIO class instead of ImageIO2 for DICOM" ON)
  option(MITK_USE_DCMTK "Use DCMTK in MITK" ON)
  option(MITK_USE_QT "Use Trolltech's Qt library in MITK" ON)
  option(MITK_USE_Boost "Use the Boost library in MITK" OFF)
  option(MITK_USE_OpenCV "Use Intel's OpenCV library" OFF)
  
    
  mark_as_advanced(MITK_USE_SUPERBUILD
                   MITK_BUILD_ALL_PLUGINS
                   )

  set(mitk_cmake_boolean_args
    MITK_USE_SUPERBUILD
    MITK_USE_BLUEBERRY
    MITK_BUILD_TUTORIAL
    MITK_BUILD_ALL_PLUGINS
    MITK_USE_CTK
    MITK_USE_GDCMIO
    MITK_USE_DCMTK
    MITK_USE_QT
    MITK_USE_Boost
    MITK_USE_OpenCV
   )
  
  #-----------------------------------------------------------------------------
  # Create the final variable containing superbuild boolean args
  #-----------------------------------------------------------------------------

  set(mitk_boolean_args)
  foreach(mitk_cmake_arg ${mitk_cmake_boolean_args})
    list(APPEND mitk_boolean_args -D${mitk_cmake_arg}:BOOL=${${mitk_cmake_arg}})
  endforeach()

  #-----------------------------------------------------------------------------
  # Additional MITK CMake variables
  #-----------------------------------------------------------------------------

  set(additional_mitk_cmakevars )
  foreach(proj CTK DCMTK VTK ITK OpenCV)
    if(${proj}_DIR)
      list(APPEND additional_mitk_cmakevars "-D${proj}_DIR:PATH=${${proj}_DIR}")
    endif()
  endforeach()

  if(MITK_USE_QT AND QT_QMAKE_EXECUTABLE)
    list(APPEND additional_mitk_cmakevars "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}")
  endif()

  if(BOOST_ROOT)
    list(APPEND additional_mitk_cmakevars "-DBOOST_ROOT:INTERNAL=${BOOST_ROOT}")
  endif()
  
  if(MITK_INITIAL_CACHE_FILE)
    list(APPEND additional_mitk_cmakevars "-DMITK_INITIAL_CACHE_FILE:INTERNAL=${MITK_INITIAL_CACHE_FILE}")
  endif()

  if(MITK_USE_SUPERBUILD)
    set(MITK_BINARY_DIR ${proj}-superbuild)
  else()
    set(MITK_BINARY_DIR ${proj}-build)
  endif()
  
  set(proj_DEPENDENCIES)
  set(MITK_DEPENDS ${proj})

  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${MITK_BINARY_DIR}
    PREFIX ${proj}${ep_suffix}
    GIT_REPOSITORY "${git_protocol}://git.mitk.org/MITK.git"
    GIT_TAG "origin/master"
    INSTALL_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${ep_common_args}
      ${mitk_boolean_args}
      ${additional_mitk_cmakevars}
      -DBUILD_SHARED_LIBS:BOOL=ON
    DEPENDS
      ${proj_DEPENDENCIES}
    )
  
  if(MITK_USE_SUPERBUILD)
    set(MITK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${MITK_BINARY_DIR}/MITK-build")
  else()
    set(MITK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${MITK_BINARY_DIR}")
  endif()

else()

  # The project is provided using MITK_DIR, nevertheless since other 
  # projects may depend on MITK, let's add an 'empty' one
  MacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
    
endif()

