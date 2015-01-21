#-----------------------------------------------------------------------------
# redland
#-----------------------------------------------------------------------------

if(MITK_USE_redland)

# Sanity checks
if(DEFINED redland_DIR AND NOT EXISTS ${redland_DIR})
  message(FATAL_ERROR "redland_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj redland)
set(proj_DEPENDENCIES ${raptor2_DEPENDS} ${rasqal_DEPENDS})
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED redland_DIR)

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/redland-1.0.17.tar.gz
     URL_MD5 e5be03eda13ef68aabab6e42aa67715e
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/redland-1.0.17.patch
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       -DWITH_THREADS:BOOL=OFF
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
