#-----------------------------------------------------------------------------
# Breakpad
#-----------------------------------------------------------------------------

if(MITK_USE_BREAKPAD)

  # Sanity checks
  if(DEFINED Breakpad_DIR AND NOT EXISTS ${Breakpad_DIR})
    message(FATAL_ERROR "Breakpad_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Breakpad)
  set(proj_DEPENDENCIES )
  set(Breakpad_DEPENDS ${proj})

  if(NOT DEFINED Breakpad_DIR)

    set(revision_tag 1140)

    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      #URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/BREAKPAD_${revision_tag}.tar.gz
      #URL_MD5 70fadbc62ec5c7d83a6e672fd5be36b1
      SVN_REPOSITORY http://google-breakpad.googlecode.com/svn/trunk
      SVN_REVISION -r ${revision_tag}
      PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/CMakeExternals/build-breakpad.cmake <SOURCE_DIR>/CMakeLists.txt
      UPDATE_COMMAND ""
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
      DEPENDS ${proj_DEPENDENCIES}
     )
  set(Breakpad_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
  set(Breakpad_SRC ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
