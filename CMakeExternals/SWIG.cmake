#------------------------------------------------------------
# SWIG (Simple Wrapper Interface Generator)
#-----------------------------------------------------------
if(MITK_USE_SWIG)
  if(DEFINED SWIG_DIR AND NOT EXISTS ${SWIG_DIR})
    message(FATAL_ERROR "SWIG_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT SWIG_DIR)

    # We don't "install" SWIG in the common install prefix,
    # since it is only used as a tool during the MITK super-build
    # to generate the Python wrappings for some projects.

    set(SWIG_TARGET_VERSION 3.0.2)
    set(proj SWIG)
    set(SWIG_DEPENDENCIES PCRE)
    set(SWIG_DEPENDS ${proj})

    # binary SWIG for windows
    if(WIN32)
      set(swig_source_dir ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION})

      # swig.exe available as pre-built binary on Windows:
      ExternalProject_Add(${proj}
        URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/swigwin-${SWIG_TARGET_VERSION}.zip
        URL_MD5 "3f18de4fc09ab9abb0d3be37c11fbc8f"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        )

      ExternalProject_Get_Property(${proj} source_dir)
      set(SWIG_DIR ${source_dir})
      set(SWIG_EXECUTABLE ${source_dir}/swig.exe)

    else()

      list(APPEND SWIG_DEPENDENCIES PCRE)

      # swig uses bison find it by cmake and pass it down
      find_package(BISON)
      set(BISON_FLAGS "" CACHE STRING "Flags used by bison")
      mark_as_advanced( BISON_FLAGS)

      ExternalProject_add(${proj}
        LIST_SEPARATOR ${sep}
        URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/swig-${SWIG_TARGET_VERSION}.tar.gz
        # Custom install dir for SWIG
        INSTALL_DIR ${ep_prefix}/src/${proj}-install
        URL_MD5 "62f9b0d010cef36a13a010dc530d0d41"
        CONFIGURE_COMMAND <SOURCE_DIR>/./configure
                          CC=${CMAKE_C_COMPILER}${CMAKE_C_COMPILER_ARG1}
                          LDFLAGS=${CMAKE_LINKER_FLAGS} ${CMAKE_LINKER_FLAGS_RELEASE}
                          CXX=${CMAKE_CXX_COMPILER}${CMAKE_CXX_COMPILER_ARG1}
                            --prefix=<INSTALL_DIR>
                            --with-pcre-prefix=${PCRE_DIR}
                            --without-octave
                            --with-python=${PYTHON_EXECUTABLE}
        DEPENDS ${SWIG_DEPENDENCIES}
        )

      ExternalProject_Get_Property(${proj} install_dir)
      set(SWIG_DIR ${install_dir}/share/swig/${SWIG_TARGET_VERSION})
      set(SWIG_EXECUTABLE ${install_dir}/bin/swig)

    endif()
  endif()
endif()
