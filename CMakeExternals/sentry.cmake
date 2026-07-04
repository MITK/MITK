set(proj sentry)
set(proj_DEPENDENCIES ZLIB)

if(MITK_USE_${proj})
  set(${proj}_DEPENDS ${proj})

  if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory!")
  endif()

  if(NOT DEFINED ${proj}_DIR)
    # Static build: exports sentry_crashpad::client (the only route to
    # non-fatal DumpWithoutCrash snapshots) and avoids shipping a sentry
    # shared library. Transport "none" compiles out sentry's own event
    # upload path; together with never setting a DSN the crash handler
    # is started with an empty upload URL, so dumps stay local.
    set(cmake_cache_args
      ${ep_common_cache_args}
      -DSENTRY_BACKEND:STRING=crashpad
      -DSENTRY_TRANSPORT:STRING=none
      -DSENTRY_BUILD_SHARED_LIBS:BOOL=OFF
      -DSENTRY_BUILD_TESTS:BOOL=OFF
      -DSENTRY_BUILD_EXAMPLES:BOOL=OFF
      # Crashpad client headers are not installed by default when crashpad
      # is built as a subdirectory of sentry-native. Set up front: enabling
      # it later would force an external-project rebuild.
      -DCRASHPAD_ENABLE_INSTALL_DEV:BOOL=ON
    )

    ExternalProject_Add(${proj}
      GIT_REPOSITORY https://github.com/getsentry/sentry-native.git
      GIT_TAG fea16b84ef4e723dd3a0e7f5e76f1737973dd349 # 0.15.2
      CMAKE_ARGS ${ep_common_args}
      CMAKE_CACHE_ARGS ${cmake_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(${proj}_DIR ${ep_prefix})
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
