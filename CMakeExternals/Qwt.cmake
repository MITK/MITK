#-----------------------------------------------------------------------------
# Qwt
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED Qwt_DIR AND NOT EXISTS ${Qwt_DIR})
  message(FATAL_ERROR "Qwt_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Qwt)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED ${proj}_DIR)

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/qwt-cmake-6.1.0.tar.gz
     URL_MD5 0f6e5c5221ffbbf7d0ba9a778f3e6f05
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
