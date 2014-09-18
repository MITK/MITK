#-----------------------------------------------------------------------------
# Numpy
#-----------------------------------------------------------------------------
if( MITK_USE_Python AND NOT MITK_USE_SYSTEM_PYTHON )
  # Sanity checks
  if(DEFINED Numpy_DIR AND NOT EXISTS ${Numpy_DIR})
    message(FATAL_ERROR "Numpy_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if( NOT DEFINED Numpy_DIR )
    set(proj Numpy)
    set(${proj}_DEPENDENCIES Python)
    set(Numpy_DEPENDS ${proj})

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
        set(ENV{CXXFLAGS} \"${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}\")
        set(ENV{LDFLAGS} \"${CMAKE_LINKER_FLAGS} ${CMAKE_LINKER_FLAGS_RELEASE}\")

        ")

    set(_numpy_build_step ${MITK_SOURCE_DIR}/CMake/mitkFunctionExternalPythonBuildStep.cmake)

    set(_configure_step ${CMAKE_BINARY_DIR}/${proj}-cmake/${proj}_configure_step.cmake)
    file(WRITE ${_configure_step}
       "${_numpy_env}
        include(\"${_numpy_build_step}\")
        file(WRITE \"${CMAKE_BINARY_DIR}/${proj}-src/site.cfg\" \"\")
        mitkFunctionExternalPythonBuildStep(${proj} configure \"${PYTHON_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}\" setup.py config)
       ")

    # build step
    set(_build_step ${CMAKE_BINARY_DIR}/${proj}-cmake/${proj}_build_step.cmake)
    file(WRITE ${_build_step}
       "${_numpy_env}
        include(\"${_numpy_build_step}\")
        mitkFunctionExternalPythonBuildStep(${proj} build \"${PYTHON_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}\" setup.py build --fcompiler=none)
       ")

    # install step
    set(_install_dir "${Python_DIR}")
    if(WIN32)
      STRING(REPLACE "/" "\\\\" _install_dir ${Python_DIR})
    endif()
    # escape spaces in install path
    STRING(REPLACE " " "\\ " _install_dir ${_install_dir})

    set(_install_step ${CMAKE_BINARY_DIR}/${proj}-cmake/${proj}_install_step.cmake)
    file(WRITE ${_install_step}
       "${_numpy_env}
        include(\"${_numpy_build_step}\")
        mitkFunctionExternalPythonBuildStep(${proj} install \"${PYTHON_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}\" setup.py install --prefix=${_install_dir})
       ")

    set(Numpy_URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/numpy-1.4.1.tar.gz)
    set(Numpy_MD5 "5c7b5349dc3161763f7f366ceb96516b")

    # escape spaces
    if(UNIX)
      STRING(REPLACE " " "\ " _configure_step ${_configure_step})
      STRING(REPLACE " " "\ " _build_step ${_build_step})
      STRING(REPLACE " " "\ " _install_step ${_install_step})
    endif()

    set(NUMPY_PATCH_COMMAND PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${CMAKE_CURRENT_LIST_DIR}/Patch${proj}.cmake)

    ExternalProject_Add(${proj}
      URL ${Numpy_URL}
      URL_MD5 ${Numpy_MD5}
      SOURCE_DIR ${proj}-src
      ${NUMPY_PATCH_COMMAND}
      PREFIX ${proj}-cmake
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_step}
      BUILD_COMMAND   ${CMAKE_COMMAND} -P ${_build_step}
      INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_step}

      DEPENDS
        ${${proj}_DEPENDENCIES}
    )

    set(Numpy_DIR ${MITK_PYTHON_SITE_DIR}/numpy)

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

