#-----------------------------------------------------------------------------
# GDCM
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED GDCM_DIR AND NOT EXISTS ${GDCM_DIR})
  message(FATAL_ERROR "GDCM_DIR variable is defined but corresponds to non-existing directory")
endif()

# Check if an external ITK build tree was specified.
# If yes, use the GDCM from ITK, otherwise ITK will complain
if(ITK_DIR)
  find_package(ITK)
  if(ITK_GDCM_DIR)
    set(GDCM_DIR ${ITK_GDCM_DIR})
  endif()
endif()


set(proj GDCM)
set(proj_DEPENDENCIES )
set(GDCM_DEPENDS ${proj})

if(NOT DEFINED GDCM_DIR)

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/gdcm-2.0.18.tar.gz
     URL_MD5 3c431bac0fe4da166f2b71c78f0d37a6
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
  set(GDCM_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

  set(GDCM_IS_2_0_18 TRUE)
else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  find_package(GDCM)

  if( GDCM_BUILD_VERSION EQUAL "18")
    set(GDCM_IS_2_0_18 TRUE)
  else()
    set(GDCM_IS_2_0_18 FALSE)
  endif()

endif()
