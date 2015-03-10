#-----------------------------------------------------------------------------
# ANN
#-----------------------------------------------------------------------------

if(MITK_USE_ANN)

  # Sanity checks
  if(DEFINED ANN_DIR AND NOT EXISTS ${ANN_DIR})
    message(FATAL_ERROR "ANN_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj ANN)
  set(proj_DEPENDENCIES )
  set(ANN_DEPENDS ${proj})

  if(NOT DEFINED ANN_DIR)

    set(patch_cmd ${CMAKE_COMMAND} -Dproj:STRING=${proj} -Dproj_target:STRING=ann -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake)

    ExternalProject_Add(${proj}
       LIST_SEPARATOR ${sep}
       URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/ann_1.1.2.tar.gz
       URL_MD5 7ffaacc7ea79ca39d4958a6378071365
       PATCH_COMMAND ${patch_cmd}
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(ANN_DIR ${ep_prefix}/lib/cmake/ANN)
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
