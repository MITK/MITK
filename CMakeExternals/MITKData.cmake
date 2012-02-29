#-----------------------------------------------------------------------------
# MITK Data
#-----------------------------------------------------------------------------

# Sanity checks
IF(DEFINED MITK_DATA_DIR AND NOT EXISTS ${MITK_DATA_DIR})
  MESSAGE(FATAL_ERROR "MITK_DATA_DIR variable is defined but corresponds to non-existing directory")
ENDIF()

SET(proj MITK-Data)
SET(proj_DEPENDENCIES)
SET(MITK-Data_DEPENDS ${proj})

IF(BUILD_TESTING)

  SET(revision_tag 7c47185a)
  #IF(${proj}_REVISION_TAG)
  #  SET(revision_tag ${${proj}_REVISION_TAG})
  #ENDIF()
  
  ExternalProject_Add(${proj}
    URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/MITK-Data_${revision_tag}.tar.gz
    URL_MD5 6eefdd1d792ad571cdfb152f81d6f66b
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS ${proj_DEPENDENCIES}
  )
  
  SET(MITK_DATA_DIR ${ep_source_dir}/${proj})
  
ELSE()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  
ENDIF(BUILD_TESTING)

 
