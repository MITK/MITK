#-----------------------------------------------------------------------------
# Qwt
#-----------------------------------------------------------------------------

if(MITK_USE_Qwt)

# Sanity checks
if(DEFINED Qwt_DIR AND NOT EXISTS ${Qwt_DIR})
  message(FATAL_ERROR "Qwt_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Qwt)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED ${proj}_DIR)

  set(patch_cmd ${CMAKE_COMMAND} -Dproj:STRING=${proj} -Dproj_target:STRING=qwt -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake)
  set(qt54patch_cmd ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchQwt-6.1.0.cmake)

  set(additional_cmake_args )
  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/qwt-6.1.0.tar.bz2
     URL_MD5 aef0437b37f191067a6a9dc01c30ba64
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Qwt-6.1.0.patch
       COMMAND ${CMAKE_COMMAND} -Dproj=${proj} -Dproj_target:STRING=qwt -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       ${qt_project_args}
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
