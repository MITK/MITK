if(MITK_USE_Python3)
  mitk_create_module(
    DEPENDS
      PUBLIC MitkCore
      PRIVATE MitkPythonHelper
    PACKAGE_DEPENDS
      PUBLIC nlohmann_json
      PRIVATE Boost|process
  )

  if(TARGET ${MODULE_TARGET})
    #[[ On macOS, Boost.Process would otherwise install its own SIGCHLD
        handler and break QProcess, which relies on its own. Worker exits are
        detected by the end of the protocol pipe instead. ]]
    target_compile_definitions(${MODULE_TARGET} PRIVATE BOOST_PROCESS_V2_DISABLE_SIGNALSET)

    if(APPLE)
      #[[ macOS reports POSIX shared memory as unsupported although shm_open
          works. Without this, Boost.Interprocess emulates shared memory with
          files that Python's multiprocessing.shared_memory cannot open. ]]
      target_compile_definitions(${MODULE_TARGET} PRIVATE BOOST_INTERPROCESS_POSIX_SHARED_MEMORY_OBJECTS)
    endif()

    if(BUILD_TESTING)
      add_subdirectory(test)
    endif()
  endif()
endif()
