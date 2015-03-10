set(ProjConfig.cmake.in "
set(${proj}_LIBRARIES @${proj}_LIBRARIES@)

if(NOT TARGET ${proj_target})
  include(\"\${CMAKE_CURRENT_LIST_DIR}/${proj}Targets.cmake\")
endif()
")

set(ProjConfigVersion.cmake.in "
# The created file sets PACKAGE_VERSION_EXACT if the current version string and
# the requested version string are exactly the same and it sets
# PACKAGE_VERSION_COMPATIBLE if the current version major number == requested version major number
# and the current version minor number >= requested version minor number

set(PACKAGE_VERSION_MAJOR @${proj}_MAJOR_VERSION@)
set(PACKAGE_VERSION_MINOR @${proj}_MINOR_VERSION@)
set(PACKAGE_VERSION_PATCH @${proj}_PATCH_VERSION@)
set(PACKAGE_VERSION \"@${proj}_VERSION@\")

if(PACKAGE_VERSION VERSION_EQUAL PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_EXACT TRUE)
else()
  set(PACKAGE_VERSION_EXACT FALSE)
  if(NOT PACKAGE_VERSION_MAJOR EQUAL PACKAGE_FIND_VERSION_MAJOR)
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
  elseif(PACKAGE_VERSION_MINOR LESS PACKAGE_FIND_VERSION_MINOR)
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
  else()
    set(PACKAGE_VERSION_CcOMPATIBLE TRUE)
  endif()
endif()

# if the installed or the using project don't have CMAKE_SIZEOF_VOID_P set, ignore it:
if(\"\${CMAKE_SIZEOF_VOID_P}\" STREQUAL \"\" OR \"@CMAKE_SIZEOF_VOID_P@\" STREQUAL \"\")
  return()
endif()

# check that the installed version has the same 32/64bit-ness as the one which is currently searching:
if(NOT \"\${CMAKE_SIZEOF_VOID_P}\" STREQUAL \"@CMAKE_SIZEOF_VOID_P@\")
  math(EXPR installedBits \"@CMAKE_SIZEOF_VOID_P@ * 8\")
  set(PACKAGE_VERSION \"\${PACKAGE_VERSION} (\${installedBits}bit)\")
  set(PACKAGE_VERSION_UNSUITABLE TRUE)
endif()
")

configure_file(${CMAKE_CURRENT_LIST_DIR}/${proj}CMakeLists.txt "${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt" COPYONLY)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${proj}Config.cmake.in "${ProjConfig.cmake.in}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${proj}ConfigVersion.cmake.in "${ProjConfigVersion.cmake.in}")
