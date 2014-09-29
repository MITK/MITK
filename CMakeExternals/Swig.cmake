#------------------------------------------------------------
# SWIG (Simple Wrapper Interface Generator)
#-----------------------------------------------------------
if(MITK_USE_SWIG)
  if(DEFINED Swig_DIR AND NOT EXISTS ${Swig_DIR})
    message(FATAL_ERROR "Swig_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT SWIG_DIR)
    set(SWIG_TARGET_VERSION 3.0.2)
    set(proj Swig)
    set(Swig_DEPENDENCIES )
    set(Swig_DEPENDS )

    # binary SWIG for windows
    if(WIN32)
      set(swig_source_dir ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION})

      # swig.exe available as pre-built binary on Windows:
      ExternalProject_Add(Swig
        URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/swigwin-${SWIG_TARGET_VERSION}.zip
        URL_MD5 "3f18de4fc09ab9abb0d3be37c11fbc8f"
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        )

      set(SWIG_DIR ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}) # path specified as source in ep
      set(SWIG_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}/swig.exe)

    else()

      list(APPEND Swig_DEPENDENCIES PCRE)

      # swig uses bison find it by cmake and pass it down
      find_package(BISON)
      set(BISON_FLAGS "" CACHE STRING "Flags used by bison")
      mark_as_advanced( BISON_FLAGS)

      ExternalProject_add(${proj}
        URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/swig-${SWIG_TARGET_VERSION}.tar.gz
        URL_MD5 "62f9b0d010cef36a13a010dc530d0d41"
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
        PREFIX ${proj}-cmake
        CONFIGURE_COMMAND <SOURCE_DIR>/./configure
                          CC=${CMAKE_C_COMPILER}${CMAKE_C_COMPILER_ARG1}
                          LDFLAGS=${CMAKE_LINKER_FLAGS} ${CMAKE_LINKER_FLAGS_RELEASE}
                          CXX=${CMAKE_CXX_COMPILER}${CMAKE_CXX_COMPILER_ARG1}
                            --prefix=<INSTALL_DIR>
                            --with-pcre-prefix=${PCRE_DIR}
                            --without-octave
                            --with-python=${PYTHON_EXECUTABLE}
        DEPENDS ${Swig_DEPENDENCIES}
        )

      set(SWIG_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install/share/swig/${SWIG_TARGET_VERSION})
      set(SWIG_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install/bin/swig)

    endif()
  endif(NOT SWIG_DIR)
endif()
