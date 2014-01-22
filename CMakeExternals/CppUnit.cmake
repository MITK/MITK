#-----------------------------------------------------------------------------
# CppUnit
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED CppUnit_DIR AND NOT EXISTS ${CppUnit_DIR})
  message(FATAL_ERROR "CppUnit_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj CppUnit)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED CppUnit_DIR)

  set(patch_cmd ${CMAKE_COMMAND} -Dproj=${proj} -Dproj_target:STRING=cppunit -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchCppUnit-1.12.1.cmake)

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/cppunit-1.12.1.tar.gz
     URL_MD5 bd30e9cf5523cdfc019b94f5e1d7fd19
     PATCH_COMMAND ${patch_cmd}
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
