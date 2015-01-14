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
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/gdcm-2.4.1.tar.bz2
     URL_MD5 1120f9a5ebcef7df6933ca83545f514d
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/GDCM-2.4.1.patch
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       -DGDCM_BUILD_SHARED_LIBS:BOOL=ON
       -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
     DEPENDS ${proj_DEPENDENCIES}
    )
  set(GDCM_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install/lib/gdcm-2.4)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  find_package(GDCM)

endif()
