#-----------------------------------------------------------------------------
# Numpy
#-----------------------------------------------------------------------------
if( MITK_USE_Python )

  option(MITK_USE_NUMPY "Use the numpy python library" OFF)

  # always build numpy in this case
  if(NOT MITK_USE_SYSTEM_PYTHON)
    set(MITK_USE_NUMPY ON CACHE BOOL "Use the numpy python library" FORCE)
  endif()

  if(MITK_USE_NUMPY)
    # Sanity checks
    if(DEFINED Numpy_DIR AND NOT EXISTS ${Numpy_DIR})
      message(FATAL_ERROR "Numpy_DIR variable is defined but corresponds to non-existing directory")
    endif()

    if( NOT DEFINED Numpy_DIR )
      set(proj Numpy)
      set(${proj}_DEPENDENCIES )
      set(Numpy_DEPENDS ${proj})

      if(NOT MITK_USE_SYSTEM_PYTHON)
        list(APPEND ${proj}_DEPENDENCIES Python)
      endif()

      set(_numpy_env
          "
          set(ENV{F77} \"\")
          set(ENV{F90} \"\")
          set(ENV{FFLAGS} \"\")
          set(ENV{ATLAS} \"None\")
          set(ENV{BLAS} \"None\")
          set(ENV{LAPACK} \"None\")
          set(ENV{MKL} \"None\")
          ")

      set(_external_python_project ${CMAKE_BINARY_DIR}/mitkExternalPythonProject.cmake)
      file(WRITE ${_external_python_project}
          "
          ${_numpy_env}
          set(ENV{VS_UNICODE_OUTPUT} \"\")
          set(ENV{CC} \"${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1}\")
          set(ENV{CFLAGS} \"${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}\")
          set(ENV{CXX} \"${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}\")
          set(ENV{CXXFLAGS} \"${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}\")
          set(ENV{LDFLAGS} \"${CMAKE_LINKER_FLAGS} ${CMAKE_LINKER_FLAGS_RELEASE}\")

          function(MITK_PYTHON_BUILD_STEP proj step)
             set(_command \${ARGN})

             message(\"Running \${proj} \${step}:${PYTHON_EXECUTABLE} \${_command}\")

             execute_process(
                COMMAND ${PYTHON_EXECUTABLE} \${_command}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/\${proj}-src
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_VARIABLE error
             )
             set(output_file \"${CMAKE_BINARY_DIR}/\${proj}_\${step}_step_output.txt\")
             file(WRITE \${output_file} \${output})

             set(error_file \"${CMAKE_BINARY_DIR}/\${proj}_\${step}_step_error.txt\")
             file(WRITE \${error_file} \${error})

             if(NOT \${result} EQUAL 0)
               message(FATAL_ERROR \"Error in: \${proj}: \${error}\")
             endif()
           endfunction()
          ")
      # configure step
      set(_configure_step ${CMAKE_BINARY_DIR}/${proj}_configure_step.cmake)
      file(WRITE ${_configure_step}
         "include(\"${_external_python_project}\")
          file(WRITE \"${CMAKE_BINARY_DIR}/${proj}-src/site.cfg\" \"\")
          MITK_PYTHON_BUILD_STEP(${proj} configure setup.py config)
         ")

      # build step
      set(_build_step ${CMAKE_BINARY_DIR}/${proj}_build_step.cmake)
      file(WRITE ${_build_step}
         "include(\"${_external_python_project}\")
          MITK_PYTHON_BUILD_STEP(${proj} build setup.py build --fcompiler=none)
         ")

      # install step
      set(NUMPY_CMAKE_INSTALL_DIR )
      if(NOT MITK_USE_SYSTEM_PYTHON)
        set(_install_dir ${Python_BUILD_DIR})
        if(WIN32)
          STRING(REPLACE "/" "\\\\" _install_dir ${Python_DIR})
        endif()
      else()
        set(NUMPY_CMAKE_INSTALL_DIR INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)
        set(_install_dir "${CMAKE_BINARY_DIR}/${proj}-install")
        if(WIN32)
          STRING(REPLACE "/" "\\\\" _install_dir ${CMAKE_BINARY_DIR}/${proj}-install)
        endif()
      endif()

      set(_install_step ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
      file(WRITE ${_install_step}
         "include(\"${_external_python_project}\")
          MITK_PYTHON_BUILD_STEP(${proj} install setup.py install --prefix=${_install_dir})
         ")

      set(Numpy_URL "https://dl.dropboxusercontent.com/u/8367205/ExternalProjects/numpy-1.4.1.tar.gz")
      set(Numpy_MD5 "5c7b5349dc3161763f7f366ceb96516b")

      ExternalProject_Add(${proj}
        URL ${Numpy_URL}
        URL_MD5 ${Numpy_MD5}
        SOURCE_DIR ${proj}-src
        PREFIX ${proj}-cmake
        BUILD_IN_SOURCE 1
        ${NUMPY_CMAKE_INSTALL_DIR}
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_step}
        BUILD_COMMAND   ${CMAKE_COMMAND} -P ${_build_step}
        INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_step}

        DEPENDS
          ${${proj}_DEPENDENCIES}
      )

      if(WIN32)
        set(Numpy_DIR ${_install_dir}/Lib/site-packages)
      else()
        set(Numpy_DIR ${_install_dir}/lib/python2.7/site-packages)
      endif()

      set(NUMPY_INCLUDE_DIR ${Numpy_DIR}/numpy/core/include)
      set(ENV{PYTHONPATH} "${Numpy_DIR}")
    else()
      mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
    endif()
  endif()
endif()

