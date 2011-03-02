#-----------------------------------------------------------------------------
# MITK Data
#-----------------------------------------------------------------------------

# Sanity checks
IF(DEFINED MITK_DATA_DIR AND NOT EXISTS ${MITK_DATA_DIR})
  MESSAGE(FATAL_ERROR "MITK_DATA_DIR variable is defined but corresponds to non-existing directory")
ENDIF()

IF(BUILD_TESTING)
  SET(proj MITK-Data)
  ExternalProject_Add(${proj}
    GIT_REPOSITORY ssh://git@mbits/MITK-Data
    GIT_TAG "origin/master"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
  SET(MITK_DATA_DIR ${ep_source_dir}/${proj})
ENDIF(BUILD_TESTING)
 
