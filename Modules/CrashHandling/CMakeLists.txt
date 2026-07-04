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
    #[[ The handler is renamed from upstream's generic "crashpad_handler" to
        "MitkCrashHandler" so it is recognizable as MITK's in a task manager.
        The name is defined here once and handed to the facility as a compile
        definition, so the file the build tree/installer places and the file
        the facility asks the sentry backend to spawn (handler_path) cannot
        drift apart. ]]
    set(_handler_name MitkCrashHandler${CMAKE_EXECUTABLE_SUFFIX})
    target_compile_definitions(${MODULE_TARGET} PRIVATE MITK_CRASH_HANDLER_FILENAME="${_handler_name}")

    if(MITK_EXTERNAL_PROJECT_PREFIX)
      #[[ The handler is spawned, not linked, so it has to sit in the runtime
          output directory for apps and tests launched from the build tree.
          Modeled as a file-level dependency so an external-project version
          bump propagates without a reconfigure. ]]
      set(_handler_source "${MITK_EXTERNAL_PROJECT_PREFIX}/bin/crashpad_handler${CMAKE_EXECUTABLE_SUFFIX}")

      get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
      if(_is_multi_config)
        set(_handler_copy "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/${_handler_name}")
      else()
        set(_handler_copy "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_handler_name}")
      endif()

      add_custom_command(OUTPUT "${_handler_copy}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_handler_source}" "${_handler_copy}"
        DEPENDS "${_handler_source}"
        COMMENT "Copying ${_handler_name} next to the application binaries")
      add_custom_target(MitkCopyCrashpadHandler ALL DEPENDS "${_handler_copy}")
    endif()
  endif()

  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
