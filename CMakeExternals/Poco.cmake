#-----------------------------------------------------------------------------
# Poco
#-----------------------------------------------------------------------------

if(MITK_USE_Poco)

  # Sanity checks
  if(DEFINED Poco_DIR AND NOT EXISTS ${Poco_DIR})
    message(FATAL_ERROR "Poco_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Poco)
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  if(NOT DEFINED ${proj}_DIR)

    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/poco-1.6.0-all.tar.gz
      URL_MD5 4fed893d4ca57db98b0e10d82545232a
      INSTALL_DIR ${proj}-install
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_INSTALL_NAME_DIR:STRING=<INSTALL_DIR>/lib
        -DCMAKE_INSTALL_RPATH:STRING=<INSTALL_DIR>/lib
      DEPENDS ${proj_DEPENDENCIES}
     )

    set(${proj}_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
