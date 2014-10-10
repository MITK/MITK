#-----------------------------------------------------------------------------
# SOFA
#-----------------------------------------------------------------------------

if(MITK_USE_SOFA)
  # Sanity checks
  if(DEFINED SOFA_DIR AND NOT EXISTS ${SOFA_DIR})
    message(FATAL_ERROR "SOFA_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj SOFA)
  set(proj_DEPENDENCIES Boost GLEW)
  set(SOFA_DEPENDS ${proj})

  set(preconfigure_cmake_args
    -DGLEW_DIR:PATH=${GLEW_DIR}
    -DSOFA-APPLICATION_MODELER:BOOL=OFF
    -DSOFA-APPLICATION_RUNSOFA:BOOL=OFF
    -DSOFA-APPLICATION_SOFABATCH:BOOL=OFF
    -DSOFA-EXTERNAL_BOOST:BOOL=ON
    -DSOFA-EXTERNAL_BOOST_PATH:PATH=${CMAKE_BINARY_DIR}/Boost-install/lib
    -DSOFA-EXTERNAL_CSPARSE:BOOL=ON
    -DSOFA-EXTERNAL_GLEW:BOOL=ON
    -DSOFA-EXTERNAL_PNG:BOOL=OFF
    -DSOFA-EXTERNAL_ZLIB:BOOL=OFF
    -DSOFA-LIB_COMPONENT_SPARSE_SOLVER:BOOL=ON
    -DSOFA-LIB_GUI_GLUT:BOOL=OFF
    -DSOFA-LIB_GUI_QT:BOOL=OFF
    -DSOFA-LIB_GUI_QTVIEWER:BOOL=OFF
    -DSOFA-TUTORIAL_CHAIN_HYBRID:BOOL=OFF
    -DSOFA-TUTORIAL_COMPOSITE_OBJECT:BOOL=OFF
    -DSOFA-TUTORIAL_MIXED_PENDULUM:BOOL=OFF
    -DSOFA-TUTORIAL_ONE_PARTICLE:BOOL=OFF
    -DSOFA-TUTORIAL_ONE_TETRAHEDRON:BOOL=OFF
  )

  if(NOT APPLE)
    list(APPEND proj_DEPENDENCIES GLUT)

    list(APPEND preconfigure_cmake_args
      -DSOFA-EXTERNAL_FREEGLUT:BOOL=ON
      -DGLUT_DIR:PATH=${GLUT_DIR}
    )
  endif()

  if(NOT MITK_USE_SYSTEM_Boost)
    list(APPEND preconfigure_cmake_args
      -DBoost_NO_SYSTEM_PATHS:BOOL=ON
      -DBOOST_INCLUDEDIR:PATH=${CMAKE_BINARY_DIR}/Boost-install/include
      -DBOOST_LIBRARYDIR:PATH=${CMAKE_BINARY_DIR}/Boost-install/lib
      -DBoost_ADDITIONAL_VERSIONS:STRING=1.56
    )
  endif()

  if(MITK_USE_SOFA_PLUGINS_DIR)
    list(APPEND preconfigure_cmake_args
      -DSOFA_APPLICATIONS_PLUGINS_DIR:PATH=${MITK_USE_SOFA_PLUGINS_DIR}
    )

    foreach(plugin ${MITK_USE_SOFA_PLUGINS})
      string(TOUPPER ${plugin} plugin)
      list(APPEND preconfigure_cmake_args
        -DSOFA-PLUGIN_${plugin}:BOOL=ON
      )
    endforeach()
  endif()

  set(rev "10669")

  set(SOFA_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchSOFA-rev${rev}.cmake)
  set(SOFA_PRECONFIGURE_COMMAND ${CMAKE_COMMAND} -G${gen} ${ep_common_args} ${preconfigure_cmake_args} ${boost_cmake_args} ${CMAKE_BINARY_DIR}/${proj}-src)

  if(NOT DEFINED SOFA_DIR)
    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/SOFA-rev${rev}.tar.gz
      URL_MD5 d01a194f54b933f4cdfdfc75b2f81a2f
      PATCH_COMMAND ${SOFA_PATCH_COMMAND}
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    ExternalProject_Add_Step(${proj} preconfigure
      COMMAND ${SOFA_PRECONFIGURE_COMMAND}
      WORKING_DIRECTORY ${proj}-build
      DEPENDEES patch
      DEPENDERS configure
      LOG 1
    )

    set(SOFA_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
