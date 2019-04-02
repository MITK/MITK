#-----------------------------------------------------------------------------
# raptor2
#-----------------------------------------------------------------------------

if(MITK_USE_Raptor2)

# Sanity checks
if(DEFINED Raptor2_DIR AND NOT EXISTS ${Raptor2_DIR})
  message(FATAL_ERROR "Raptor2_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Raptor2)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED Raptor2_DIR)

  set(additional_cmake_args )
  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/raptor2-2.0.15.tar.gz
     URL_MD5 a39f6c07ddb20d7dd2ff1f95fa21e2cd
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Raptor2-2.0.15.patch
     CMAKE_GENERATOR ${gen}
     CMAKE_GENERATOR_PLATFORM ${gen_platform}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       -DRAPTOR_ENABLE_TESTING:BOOL=OFF
       -DRAPTOR_PARSER_GRDDL:BOOL=OFF
       -DRAPTOR_PARSER_GUESS:BOOL=OFF
       -DRAPTOR_PARSER_JSON:BOOL=OFF
       -DRAPTOR_PARSER_NQUADS:BOOL=ON
       -DRAPTOR_PARSER_NTRIPLES:BOOL=ON
       -DRAPTOR_PARSER_RDFA:BOOL=OFF
       -DRAPTOR_PARSER_RDFXML:BOOL=OFF
       -DRAPTOR_PARSER_RSS:BOOL=OFF
       -DRAPTOR_PARSER_TRIG:BOOL=OFF
       -DRAPTOR_PARSER_TURTLE:BOOL=ON
       -DRAPTOR_SERIALIZER_ATOM:BOOL=OFF
       -DRAPTOR_SERIALIZER_DOT:BOOL=OFF
       -DRAPTOR_SERIALIZER_HTML:BOOL=OFF
       -DRAPTOR_SERIALIZER_JSON:BOOL=OFF
       -DRAPTOR_SERIALIZER_NQUADS:BOOL=ON
       -DRAPTOR_SERIALIZER_NTRIPLES:BOOL=ON
       -DRAPTOR_SERIALIZER_RDFXML:BOOL=OFF
       -DRAPTOR_SERIALIZER_RDFXML_ABBREV:BOOL=OFF
       -DRAPTOR_SERIALIZER_RSS_1_0:BOOL=OFF
       -DRAPTOR_SERIALIZER_TURTLE:BOOL=ON
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix}/lib/raptor2/cmake)
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
