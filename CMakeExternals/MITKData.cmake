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

  if(DEFINED MITK_DATA_DIR)

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  else()

  # Pin mitk-data to the parent of its commit "Update file to reflect changes
  # introduced by #800". That commit tailors the liver-spine labelmap reference
  # data to #800, but #800 depends on this branch (#323) landing first. Until
  # then #323 must build against the pre-#800 reference data; otherwise the
  # DICOM Seg labelmap regression test fails.
  # TODO: Revert this pin once #800 has landed, i.e. remove GIT_TAG again so
  #       mitk-data is tracked at HEAD (or advance it to the #800 commit).
  set(revision_tag a1e677b4) # first 8 characters of hash-tag
  #                  ^^^^^^^^  these are just to check correct length of hash part

    ExternalProject_Add(${proj}
      SOURCE_DIR ${proj}
      GIT_REPOSITORY https://codebase.helmholtz.cloud/mitk/mitk-data.git
      GIT_TAG ${revision_tag}
  #   URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/mitk-data_${revision_tag}.tar.gz
  #   UPDATE_COMMAND ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(MITK_DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj})

  endif()

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
