#-----------------------------------------------------------------------------
# Installation
#-----------------------------------------------------------------------------

install(TARGETS ${exec_target} DESTINATION .)

install(FILES "${PROJECT_SOURCE_DIR}/Changelog.txt" DESTINATION .)

install(CODE "
  set(DIRS
      ${QT_LIBRARY_DIR}
      ${QT_LIBRARY_DIR}/../bin
      ${CTK_LIBRARY_DIRS}
     )

  include(BundleUtilities)

  fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${exec_target}${CMAKE_EXECUTABLE_SUFFIX}\" \"\" \"\${DIRS}\")
")

#-----------------------------------------------------------------------------
# Packaging
#-----------------------------------------------------------------------------

#
# First, set the generator variable
#
if(WIN32)
  set(CPACK_GENERATOR ZIP)
elseif(APPLE)
  set(CPACK_GENERATOR DragNDrop)
else()
  set(CPACK_GENERATOR TGZ)
endif()

# include required mfc libraries
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "MITKPluginGenerator")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MITK PluginGenerator bootstraps MITK-based projects")
set(CPACK_PACKAGE_VENDOR "German Cancer Research Center (DKFZ)")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")

# tell cpack to strip all debug symbols from all files
set(CPACK_STRIP_FILES ON)

include(CPack)

