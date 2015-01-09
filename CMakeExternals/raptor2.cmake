#-----------------------------------------------------------------------------
# raptor2
#-----------------------------------------------------------------------------

if(MITK_USE_raptor2)

# Sanity checks
if(DEFINED raptor2_DIR AND NOT EXISTS ${raptor2_DIR})
  message(FATAL_ERROR "raptor2_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj raptor2)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED raptor2_DIR)

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     INSTALL_DIR ${REDLAND_INSTALL_DIR}
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/raptor2-2.0.15.tar.gz
     URL_MD5 a39f6c07ddb20d7dd2ff1f95fa21e2cd
     PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_LIST_DIR}/raptor2-2.0.15.patch
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
       -DBUILD_SHARED_LIBS:BOOL=ON
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
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${REDLAND_INSTALL_DIR}/lib/raptor2/cmake/)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
