#-----------------------------------------------------------------------------
# ACVD
#-----------------------------------------------------------------------------

if(MITK_USE_ACVD)
  # Sanity checks
  if(DEFINED ACVD_DIR AND NOT EXISTS ${ACVD_DIR})
    message(FATAL_ERROR "ACVD_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj ACVD)
  set(proj_DEPENDENCIES VTK)
  set(ACVD_DEPENDS ${proj})

  set(additional_cmake_args
    -DUSE_MULTITHREADING:BOOL=ON
    -DVTK_DIR:PATH=${VTK_DIR}
  )

  set(ACVD_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchACVD.cmake)

  if(NOT DEFINED ACVD_DIR)
    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      URL http://mitk.org/download/thirdparty/ACVD_VTK6.tar.gz
      URL_MD5 e99d1cb7d264d421074d5346f1d3ce82
      PATCH_COMMAND ${ACVD_PATCH_COMMAND}
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(ACVD_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
