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
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     INSTALL_DIR ${REDLAND_INSTALL_DIR}
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/redland-1.0.17.tar.gz
     URL_MD5 e5be03eda13ef68aabab6e42aa67715e
     PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_LIST_DIR}/redland-1.0.17.patch
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
       -DBUILD_SHARED_LIBS:BOOL=ON
       -DWITH_THREADS:BOOL=OFF
       -DCMAKE_PREFIX_PATH:STRING=${REDLAND_INSTALL_DIR}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${REDLAND_INSTALL_DIR}/lib/redland/cmake/)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
