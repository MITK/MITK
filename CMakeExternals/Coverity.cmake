#-----------------------------------------------------------------------------
# Coverity
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED Coverity_DIR AND NOT EXISTS ${Coverity_DIR})
  message(FATAL_ERROR "Coverity_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Coverity)
set(proj_DEPENDENCIES )
set(Coverity_DEPENDS ${proj})

if(NOT DEFINED Coverity_DIR)

  set(_coverity_download_url_base https://scan.coverity.com/download/cxx/)
  if(UNIX AND "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(Coverity_DOWNLOAD_URL ${_coverity_download_url_base}/linux-64)
  else()
    message(FATAL_ERROR "The Coverity superbuild integration currently only supports 64-bit Linux platforms")
  endif()

  if(NOT MITK_COVERITY_SCAN_TOKEN)
    message(FATAL_ERROR "A Coverity Scan authentication token is required to download the Coverity Scan client")
  endif()

  set(coverity_download_dest ${CMAKE_BINARY_DIR}/${proj}-cmake/src/cov-analysis-linux64-7.6.0.tar.gz)
  ExternalProject_Add(${proj}
    INSTALL_DIR ${proj}-install
    PREFIX ${CMAKE_BINARY_DIR}/${proj}-cmake
    # NOTE: This URL will download the latest available version and will
    # therefore break when coverity updates to a newer version (the MD5 will
    # change and the version number in the installation directory will
    # change). Coverity claims to udpate this download every ~6 months.
    DOWNLOAD_COMMAND wget https://scan.coverity.com/download/cxx/linux-64
        --post-data "token=${MITK_COVERITY_SCAN_TOKEN}&project=MITK"
        -O ${coverity_download_dest}
      COMMAND echo "71b5521cf4c1f549a0bcdb1646e554cf ${coverity_download_dest}" | md5sum --check
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir <INSTALL_DIR> ${CMAKE_COMMAND} -E tar xzf ${coverity_download_dest}
    DEPENDS ${proj_DEPENDENCIES}
  )

  set(Coverity_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install/cov-analysis-linux64-7.6.0)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
