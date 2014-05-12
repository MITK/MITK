#-----------------------------------------------------------------------------
# SimpleITK
#-----------------------------------------------------------------------------

if(MITK_USE_SimpleITK)

  # Sanity checks
  if(DEFINED SimpleITK_DIR AND NOT EXISTS ${SimpleITK_DIR})
    message(FATAL_ERROR "SimpleITK_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj SimpleITK)
  set(proj_DEPENDENCIES ITK GDCM)

  if(MITK_USE_OpenCV)
    list(APPEND proj_DEPENDENCIES OpenCV)
  endif()

  set(SimpleITK_DEPENDS ${proj})

  if(NOT DEFINED SimpleITK_DIR)

    set(additional_cmake_args )

    list(APPEND additional_cmake_args
         -DWRAP_CSHARP:BOOL=OFF
         -DWRAP_TCL:BOOL=OFF
         -DWRAP_LUA:BOOL=OFF
         -DWRAP_PYTHON:BOOL=OFF
        )

    if(MITK_USE_Python)
      list(APPEND additional_cmake_args
        -DWRAP_PYTHON:BOOL=ON
          )
    endif(MITK_USE_Python)

    if(MITK_USE_OpenCV)
      list(APPEND additional_cmake_args
           -DOpenCV_DIR:PATH=${OpenCV_DIR}
          )
    endif()

    set(SimpleITK_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchSimpleITK.cmake)

    ExternalProject_Add(${proj}
       SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
       BINARY_DIR ${proj}-build
       PREFIX ${proj}-cmake
       GIT_REPOSITORY "http://itk.org/SimpleITK.git"
       GIT_TAG "493f15f5cfc620413d0aa7bb705ffe6d038a41b5"
       INSTALL_COMMAND ""
       PATCH_COMMAND ${SimpleITK_PATCH_COMMAND}
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_cmake_args}
         -DBUILD_TESTING:BOOL=OFF
         -DBUILD_EXAMPLES:BOOL=OFF
         -DGDCM_DIR:PATH=${GDCM_DIR}
         -DITK_DIR:PATH=${ITK_DIR}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(SimpleITK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()

