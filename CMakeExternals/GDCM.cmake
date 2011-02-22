#-----------------------------------------------------------------------------
# GDCM
#-----------------------------------------------------------------------------

# Sanity checks
IF(DEFINED GDCM_DIR AND NOT EXISTS ${GDCM_DIR})
  MESSAGE(FATAL_ERROR "GDCM_DIR variable is defined but corresponds to non-existing directory")
ENDIF()

SET(proj GDCM)
SET(proj_DEPENDENCIES )
SET(GDCM_DEPENDS ${proj})

IF(NOT DEFINED GDCM_DIR)
  ExternalProject_Add(${proj}
     URL http://mitk.org/download/thirdparty/gdcm-2.0.14.tar.gz 
     BINARY_DIR ${proj}-build
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       -DBUILD_SHARED_LIBS:BOOL=ON 
       -DGDCM_BUILD_SHARED_LIBS:BOOL=ON 
       -DBUILD_TESTING:BOOL=OFF
       -DBUILD_EXAMPLES:BOOL=OFF
     DEPENDS ${proj_DEPENDENCIES}
    )
  SET(GDCM_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
  
ELSE()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
   
ENDIF()
