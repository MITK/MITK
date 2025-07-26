if(MITK_USE_Python3)
  mitk_create_module(
    DEPENDS PUBLIC MitkCore
    #[[ This module must not depend on Python. It is used by the
        PreloadPython auto-load module, which loads the Python library
        at runtime as early as possible to get global symbol visibility.
        It is important that this happens before any module with a link-time
        dependency to the Python library is loaded.
     ]]
  )

  if(TARGET ${MODULE_TARGET})
    find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED MODULE)
    get_filename_component(MITK_PYTHON_LIBRARY_FILENAME "${Python3_LIBRARIES}" NAME)
    configure_file(mitkPythonHelper.h.in mitkPythonHelper.h @ONLY)
    
    if(NOT WIN32)
      add_subdirectory(autoload/PreloadPython)
    endif()
  endif()
endif()

