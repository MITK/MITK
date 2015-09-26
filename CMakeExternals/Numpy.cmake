#-----------------------------------------------------------------------------
# Numpy
#-----------------------------------------------------------------------------
if( MITK_USE_Python AND NOT MITK_USE_SYSTEM_PYTHON )
  # Sanity checks
  if(DEFINED Numpy_DIR AND NOT EXISTS ${Numpy_DIR})
    message(FATAL_ERROR "Numpy_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Numpy)
  set(proj_DEPENDENCIES Python)
  set(Numpy_DEPENDS ${proj})

  if( NOT DEFINED Numpy_DIR )

    # setup build environment and disable fortran, blas and lapack
    set(_numpy_env
        "
        set(ENV{F77} \"\")
        set(ENV{F90} \"\")
        set(ENV{FFLAGS} \"\")
        set(ENV{ATLAS} \"None\")
        set(ENV{BLAS} \"None\")
        set(ENV{LAPACK} \"None\")
        set(ENV{MKL} \"None\")
        set(ENV{VS_UNICODE_OUTPUT} \"\")
        set(ENV{CC} \"${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1}\")
        set(ENV{CFLAGS} \"${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}\")
        set(ENV{CXX} \"${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}\")
        set(ENV{CXXFLAGS} \"${MITK_CXX11_FLAG} ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}\")
        set(ENV{LDFLAGS} \"${CMAKE_LINKER_FLAGS} ${CMAKE_LINKER_FLAGS_RELEASE} ${_install_rpath_linkflag}\")

        ")

    set(_numpy_build_step ${MITK_SOURCE_DIR}/CMake/mitkFunctionExternalPythonBuildStep.cmake)

    set(_configure_step ${ep_prefix}/tmp/${proj}_configure_step.cmake)
    file(WRITE ${_configure_step}
       "${_numpy_env}
        include(\"${_numpy_build_step}\")
        file(WRITE \"${ep_prefix}/src/${proj}/site.cfg\" \"\")
        mitkFunctionExternalPythonBuildStep(${proj} configure \"${PYTHON_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}\" setup.py config)
       ")

    set(_numpy_compiler )
    if(WIN32)
     set(_numpy_compiler --compiler=msvc)
    endif()

    # build step
    set(_build_step ${ep_prefix}/tmp/${proj}_build_step.cmake)
    file(WRITE ${_build_step}
       "${_numpy_env}
        include(\"${_numpy_build_step}\")
        mitkFunctionExternalPythonBuildStep(${proj} build \"${PYTHON_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}\" setup.py build ${_numpy_compiler})
       ")

    # install step
    set(_install_step ${ep_prefix}/tmp/${proj}_install_step.cmake)
    file(WRITE ${_install_step}
       "${_numpy_env}
        include(\"${_numpy_build_step}\")
        # escape characters in install path
        set(_install_dir \"${Python_DIR}\")
        if(WIN32)
          string(REPLACE \"/\" \"\\\\\" _install_dir \${_install_dir})
        endif()
        string(REPLACE \" \" \"\\ \" _install_dir \${_install_dir})
        mitkFunctionExternalPythonBuildStep(${proj} install \"${PYTHON_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}\" setup.py install --prefix=\${_install_dir})
       ")

    set(Numpy_URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/numpy-1.9.2.tar.gz)
    #set(Numpy_URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/numpy-1.4.1.tar.gz)
    #set(Numpy_MD5 "5c7b5349dc3161763f7f366ceb96516b")
    set(Numpy_MD5 "a1ed53432dbcd256398898d35bc8e645")

    # escape spaces
    if(UNIX)
      STRING(REPLACE " " "\ " _configure_step ${_configure_step})
      STRING(REPLACE " " "\ " _build_step ${_build_step})
      STRING(REPLACE " " "\ " _install_step ${_install_step})
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${Numpy_URL}
      URL_MD5 ${Numpy_MD5}
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_step}
      BUILD_COMMAND   ${CMAKE_COMMAND} -P ${_build_step}
      INSTALL_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -P ${_install_step}

      DEPENDS
        ${proj_DEPENDENCIES}
    )

    set(Numpy_DIR ${MITK_PYTHON_SITE_DIR}/numpy)
    install(SCRIPT ${_install_step})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

