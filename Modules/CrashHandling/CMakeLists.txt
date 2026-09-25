if(MITK_USE_sentry)
  #[[ This module must stay Qt-free (no Qt package dependencies, no QString
      in the public API): it is initialized at the top of
      mitk::BaseApplication::run(), before any QApplication exists. It is
      also the only module that may link the static sentry/crashpad
      libraries; a second linking target would duplicate process-global
      crash-handler state. ]]
  mitk_create_module(
    DEPENDS PUBLIC MitkCore
    PACKAGE_DEPENDS PRIVATE sentry
  )

  if(TARGET ${MODULE_TARGET})
    #[[ crashpad_client.h (used by CaptureSnapshot) includes util/... via the
        crashpad root, which the imported sentry_crashpad::client target's
        include interface does not expose (it only adds crashpad/client and
        crashpad/mini_chromium). Add the root here so the header resolves. ]]
    if(MITK_EXTERNAL_PROJECT_PREFIX)
      target_include_directories(${MODULE_TARGET} PRIVATE
        "${MITK_EXTERNAL_PROJECT_PREFIX}/include/crashpad")
    endif()

    #[[ The handler is renamed from upstream's generic "crashpad_handler" to
        "MitkCrashHandler" so it is recognizable as MITK's in a task manager.
        Handing the name defined by the top-level CMakeLists to the facility as
        a compile definition keeps the file the build tree/installer places and
        the file the facility asks the sentry backend to spawn (handler_path)
        from drifting apart. ]]
    target_compile_definitions(${MODULE_TARGET} PRIVATE
      MITK_CRASH_HANDLER_FILENAME="${MITK_CRASH_HANDLER_NAME}")

    if(MITK_CRASH_HANDLER_EXECUTABLE)
      #[[ The handler is spawned, not linked, so it has to sit in the runtime
          output directory for the plain binaries launched from the build tree.
          Application bundles are served by
          mitkFunctionCreateBlueBerryApplication, whose executables live inside
          the bundle rather than here. Modeled as a file-level dependency so an
          external-project version bump propagates without a reconfigure. ]]
      get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
      if(_is_multi_config)
        set(_handler_copy "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/${MITK_CRASH_HANDLER_NAME}")
      else()
        set(_handler_copy "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MITK_CRASH_HANDLER_NAME}")
      endif()

      add_custom_command(OUTPUT "${_handler_copy}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${MITK_CRASH_HANDLER_EXECUTABLE}" "${_handler_copy}"
        DEPENDS "${MITK_CRASH_HANDLER_EXECUTABLE}"
        COMMENT "Copying ${MITK_CRASH_HANDLER_NAME} next to the application binaries")
      add_custom_target(MitkCopyCrashpadHandler ALL DEPENDS "${_handler_copy}")
    endif()
  endif()

  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
