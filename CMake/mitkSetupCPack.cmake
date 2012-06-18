#
# First, set the generator variable
#
if(NOT CPACK_GENERATOR)
  if(WIN32)
    find_program(NSIS_MAKENSIS NAMES makensis
      PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
      DOC "Where is makensis.exe located"
      )

    if(NOT NSIS_MAKENSIS)
      set(CPACK_GENERATOR ZIP)
    else()
      set(CPACK_GENERATOR "NSIS;ZIP")
      
    endif(NOT NSIS_MAKENSIS)
  else()
    if(APPLE)
      set(CPACK_GENERATOR DragNDrop)
    else()
      set(CPACK_GENERATOR TGZ)
    endif()
  endif()
endif(NOT CPACK_GENERATOR)

# include required mfc libraries
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "MITK")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MITK is a medical image processing tool")
set(CPACK_PACKAGE_VENDOR "German Cancer Research Center (DKFZ)")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${MITK_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${MITK_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${MITK_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MITK_VERSION_MINOR}")

# tell cpack to strip all debug symbols from all files
set(CPACK_STRIP_FILES ON)

# append revision number if available
if(MITK_REVISION_ID AND MITK_VERSION_PATCH STREQUAL "99")
  if(MITK_WC_TYPE STREQUAL "git")
    set(git_hash ${MITK_REVISION_ID})
    string(LENGTH "${git_hash}" hash_length)
    if(hash_length GREATER 6)
      string(SUBSTRING ${git_hash} 0 6 git_hash)
    endif()
    set(CPACK_PACKAGE_VERSION_PATCH "${MITK_VERSION_PATCH}_r${git_hash}")
  else()
    set(CPACK_PACKAGE_VERSION_PATCH "${MITK_VERSION_PATCH}_r${MITK_REVISION_ID}")
  endif()
else()
  set(CPACK_PACKAGE_VERSION_PATCH "${MITK_VERSION_PATCH}")
endif()



