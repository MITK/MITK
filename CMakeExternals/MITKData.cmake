#-----------------------------------------------------------------------------
# MITK Data
#-----------------------------------------------------------------------------

# Sanity checks
IF(DEFINED MITK_DATA_DIR AND NOT EXISTS ${MITK_DATA_DIR})
  MESSAGE(FATAL_ERROR "MITK_DATA_DIR variable is defined but corresponds to non-existing directory")
ENDIF()

SET(MITK-Data_DEPENDS)

IF(BUILD_TESTING)
  SET(proj MITK-Data)
  SET(proj_DEPENDENCIES)
  SET(MITK-Data_DEPENDS ${proj})

  ExternalProject_Add(${proj}
    GIT_REPOSITORY http://git.mitk.org/MITK-Data.git
    GIT_TAG "origin/master"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS ${proj_DEPENDENCIES}
  )
  
  SET(MITK_DATA_DIR ${ep_source_dir}/${proj})
  
ELSE()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  
ENDIF(BUILD_TESTING)

 
