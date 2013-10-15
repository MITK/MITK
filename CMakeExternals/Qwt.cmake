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

  set(patch_cmd ${CMAKE_COMMAND} -Dproj:STRING=${proj} -Dproj_target:STRING=qwt -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake)

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/qwt-6.1.0.tar.bz2
     URL_MD5 aef0437b37f191067a6a9dc01c30ba64
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
