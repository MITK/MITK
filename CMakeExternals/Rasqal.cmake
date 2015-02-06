#-----------------------------------------------------------------------------
# rasqal
#-----------------------------------------------------------------------------

if(MITK_USE_Rasqal)

# Sanity checks
if(DEFINED Rasqal_DIR AND NOT EXISTS ${Rasqal_DIR})
  message(FATAL_ERROR "Rasqal_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Rasqal)
set(proj_DEPENDENCIES ${Raptor2_DEPENDS} ${PCRE_DEPENDS})
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED Rasqal_DIR)

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/rasqal-0.9.32.tar.gz
     URL_MD5 dc7c6107de00c47f85f6ab7db164a136
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Rasqal-0.9.32.patch
     LIST_SEPARATOR ^^
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       "-DCMAKE_C_FLAGS:STRING=-DPCRE_STATIC ${CMAKE_C_FLAGS}"
       -DRASQAL_REGEX:STRING=pcre
       -DCMAKE_PREFIX_PATH:STRING=${PCRE_DIR}^^${REDLAND_INSTALL_DIR}
       -DPCRE_INCLUDE_DIR:PATH=${PCRE_DIR}/include
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix}/lib/rasqal/cmake)
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
