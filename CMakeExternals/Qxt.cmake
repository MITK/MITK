#-----------------------------------------------------------------------------
# Qxt
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED Qxt_DIR AND NOT EXISTS ${Qxt_DIR})
  message(FATAL_ERROR "Qxt_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Qxt)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED ${proj}_DIR)

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/libqxt-cmake-0.6.2.tar.gz
     URL_MD5 345d6282baf64c8a0c35eeb6e560c0b2
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
