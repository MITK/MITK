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

  set(revision_tag f04939c6) # first 8 characters of hash-tag
#                  ^^^^^^^^  these are just to check correct length of hash part

  ExternalProject_Add(${proj}
    URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/MITK-Data_${revision_tag}.tar.gz
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


