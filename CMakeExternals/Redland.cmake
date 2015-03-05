#-----------------------------------------------------------------------------
# redland
#-----------------------------------------------------------------------------

if(MITK_USE_Redland)

# Sanity checks
if(DEFINED Redland_DIR AND NOT EXISTS ${Redland_DIR})
  message(FATAL_ERROR "Redland_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Redland)
set(proj_DEPENDENCIES ${Raptor2_DEPENDS} ${Rasqal_DEPENDS})
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED Redland_DIR)

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/redland-1.0.17.tar.gz
     URL_MD5 e5be03eda13ef68aabab6e42aa67715e
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Redland-1.0.17.patch
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       -DWITH_THREADS:BOOL=OFF
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix}/lib/redland/cmake)
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
