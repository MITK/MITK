#-----------------------------------------------------------------------------
# rasqal
#-----------------------------------------------------------------------------

if(MITK_USE_rasqal)

# Sanity checks
if(DEFINED rasqal_DIR AND NOT EXISTS ${rasqal_DIR})
  message(FATAL_ERROR "rasqal_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj rasqal)
set(proj_DEPENDENCIES ${raptor2_DEPENDS} ${PCRE_DEPENDS})
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED rasqal_DIR)

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     INSTALL_DIR ${REDLAND_INSTALL_DIR}
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/rasqal-0.9.32.tar.gz
     URL_MD5 dc7c6107de00c47f85f6ab7db164a136
     PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_LIST_DIR}/rasqal-0.9.32.patch
     LIST_SEPARATOR ^^
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
       -DBUILD_SHARED_LIBS:BOOL=ON
       "-DCMAKE_C_FLAGS:STRING=-DPCRE_STATIC ${CMAKE_C_FLAGS}"
       -DRASQAL_REGEX:STRING=pcre
       -DCMAKE_PREFIX_PATH:STRING=${PCRE_DIR}^^${REDLAND_INSTALL_DIR}
       -DPCRE_INCLUDE_DIR:PATH=${PCRE_DIR}/include
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${REDLAND_INSTALL_DIR}/lib/rasqal/cmake/)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
