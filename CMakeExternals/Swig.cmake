
if(MITK_USE_SWIG)
  # Sanity checks
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
        URL https://dl.dropboxusercontent.com/u/8367205/ExternalProjects/swigwin-${SWIG_TARGET_VERSION}.zip
        URL_MD5 "3f18de4fc09ab9abb0d3be37c11fbc8f"
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        )

      set(SWIG_DIR ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}) # path specified as source in ep
      set(SWIG_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}/swig.exe)

    else()

      # swig uses bison find it by cmake and pass it down
      find_package(BISON)
      set(BISON_FLAGS "" CACHE STRING "Flags used by bison")
      mark_as_advanced( BISON_FLAGS)


      # follow the standard EP_PREFIX locations
      set(swig_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src)
      set(swig_source_dir ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src)
      set(swig_install_dir ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install)

      # configure step
      configure_file(
        ${MITK_SOURCE_DIR}/CMakeExternals/swig_configure_step.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/swig_configure_step.cmake
        @ONLY)
      set(swig_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/swig_configure_step.cmake)

      ExternalProject_add(Swig
        URL https://dl.dropboxusercontent.com/u/8367205/ExternalProjects/swig-${SWIG_TARGET_VERSION}.tar.gz
        URL_MD5 "62f9b0d010cef36a13a010dc530d0d41"
        SOURCE_DIR ${swig_source_dir}
        INSTALL_DIR ${swig_install_dir}
        PREFIX ${proj}-cmake
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${swig_CONFIGURE_COMMAND}
        )

      set(SWIG_DIR ${swig_install_dir}/share/swig/${SWIG_TARGET_VERSION})
      set(SWIG_EXECUTABLE ${swig_install_dir}/bin/swig)

    endif()
  endif(NOT SWIG_DIR)
endif()
