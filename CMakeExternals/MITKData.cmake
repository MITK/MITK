#-----------------------------------------------------------------------------
# MITK Data
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED MITK_DATA_DIR AND NOT EXISTS ${MITK_DATA_DIR})
  message(FATAL_ERROR "MITK_DATA_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj MITK-Data)
set(proj_DEPENDENCIES)
set(MITK-Data_DEPENDS ${proj})

if(BUILD_TESTING)

  set(revision_tag d01e488b)
  #if(${proj}_REVISION_TAG)
  #  set(revision_tag ${${proj}_REVISION_TAG})
  #endif()

  ExternalProject_Add(${proj}
    URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/MITK-Data_${revision_tag}.tar.gz
    URL_MD5 c7e6573a559d70363231c22ba7eabcb8 
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS ${proj_DEPENDENCIES}
  )
  
  set(MITK_DATA_DIR ${ep_source_dir}/${proj})
  
else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  
endif(BUILD_TESTING)

 
