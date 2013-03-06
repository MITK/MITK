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
     URL http://dl.dropbox.com/u/5822501/gdcm-2.2.1.tar.gz
     URL_MD5 67c6808a6f62796954f4131e24aaab2c
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       -DGDCM_BUILD_SHARED_LIBS:BOOL=ON
     DEPENDS ${proj_DEPENDENCIES}
    )
  set(GDCM_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  find_package(GDCM)

endif()
