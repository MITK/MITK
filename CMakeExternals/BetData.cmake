#-----------------------------------------------------------------------------
# Brain Extraction Data
#-----------------------------------------------------------------------------

if(MITK_USE_BetData)


# Sanity checks
if(DEFINED BetData_DIR AND NOT EXISTS ${BetData_DIR})
  message(FATAL_ERROR "BetData_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj BetData)
set(proj_DEPENDENCIES)
set(BetData_DEPENDS ${proj})

if(NOT DEFINED BetData_DIR)

  ExternalProject_Add(${proj}
    URL http://mitk.org/download/data/MITK-BET-20171220.tar.gz
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS ${proj_DEPENDENCIES}
  )

  set(BetData_DIR ${ep_prefix}/src/${proj}/)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/${proj}.h.in ${CMAKE_BINARY_DIR}/${proj}.h)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
endif()

endif()
