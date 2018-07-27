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

# set(revision_tag da5dd4ff) # first 8 characters of hash-tag
#                  ^^^^^^^^  these are just to check correct length of hash part

  ExternalProject_Add(${proj}
    SOURCE_DIR ${proj}
    GIT_REPOSITORY https://phabricator.mitk.org/source/mitkdata.git
#   GIT_TAG ${revision_tag}
#   URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/mitk-data_${revision_tag}.tar.gz
#   UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS ${proj_DEPENDENCIES}
  )

  set(MITK_DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif(BUILD_TESTING)
