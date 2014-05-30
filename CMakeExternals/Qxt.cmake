#-----------------------------------------------------------------------------
# Qxt
#-----------------------------------------------------------------------------

if(MITK_USE_Qxt)

# Sanity checks
if(DEFINED Qxt_DIR AND NOT EXISTS ${Qxt_DIR})
  message(FATAL_ERROR "Qxt_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Qxt)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED ${proj}_DIR)

  set(patch_cmd ${CMAKE_COMMAND} -Dproj:STRING=${proj} -Dproj_target:STRING=QxtCore -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake)

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/libqxt-3e7424f842d4.tar.gz
     URL_MD5 a41a1e6d114cdabfc655f278176ca062
     PATCH_COMMAND ${patch_cmd}
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       ${qt_project_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
