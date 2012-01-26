#-----------------------------------------------------------------------------
# GDCM
#-----------------------------------------------------------------------------

# Sanity checks
IF(DEFINED GDCM_DIR AND NOT EXISTS ${GDCM_DIR})
  MESSAGE(FATAL_ERROR "GDCM_DIR variable is defined but corresponds to non-existing directory")
ENDIF()

# Check if an external ITK build tree was specified.
# If yes, use the GDCM from ITK, otherwise ITK will complain
if(ITK_DIR)
  find_package(ITK)
  if(ITK_GDCM_DIR)
    set(GDCM_DIR ${ITK_GDCM_DIR})
  endif()
endif()


SET(proj GDCM)
SET(proj_DEPENDENCIES )
SET(GDCM_DEPENDS ${proj})

IF(NOT DEFINED GDCM_DIR)

  ExternalProject_Add(${proj}
     URL http://mitk.org/download/thirdparty/gdcm-2.0.18.tar.gz 
     BINARY_DIR ${proj}-build
     INSTALL_COMMAND ""
     PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchGDCM-2.0.18.cmake 
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

  SET(GDCM_IS_2_0_18 TRUE)
ELSE()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  FIND_PACKAGE(GDCM)
  
  IF( GDCM_BUILD_VERSION EQUAL "18")
    SET(GDCM_IS_2_0_18 TRUE)
  ELSE()
    SET(GDCM_IS_2_0_18 FALSE)
  ENDIF()
   
ENDIF()
