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
    -DSOFA-APPLICATION_GENERATERIGID:BOOL=OFF
    -DSOFA-APPLICATION_MODELER:BOOL=OFF
    -DSOFA-APPLICATION_RUNSOFA:BOOL=OFF
    -DSOFA-EXTERNAL_BOOST:BOOL=ON
    -DSOFA-EXTERNAL_BOOST_PATH:PATH=${BOOST_LIBRARYDIR}
    -DSOFA-EXTERNAL_CSPARSE:BOOL=ON
    -DSOFA-EXTERNAL_GLEW:BOOL=ON
    -DSOFA-EXTERNAL_PNG:BOOL=OFF
    -DSOFA-EXTERNAL_ZLIB:BOOL=OFF
    -DSOFA-LIB_COMPONENT_SPARSE_SOLVER:BOOL=ON
    -DSOFA-LIB_GUI_GLUT:BOOL=OFF
    -DSOFA-LIB_GUI_QT:BOOL=OFF
    -DSOFA-LIB_GUI_QTVIEWER:BOOL=OFF
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
      -DBOOST_ROOT:PATH=${BOOST_ROOT}
      -DBOOST_LIBRARYDIR:PATH=${BOOST_LIBRARYDIR}
      -DBoost_ADDITIONAL_VERSIONS:STRING=1.60${sep}1.60.0
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

  set(additional_cmake_args )
  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  set(rev "386a3a7+7568b4")

  set(SOFA_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchSOFA-${rev}.cmake)
  set(SOFA_PRECONFIGURE_COMMAND ${CMAKE_COMMAND} -G${gen} ${ep_common_args} ${preconfigure_cmake_args} ${boost_cmake_args} <SOURCE_DIR>)

  if(NOT DEFINED SOFA_DIR)
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/SOFA-${rev}.tar.gz
      URL_MD5 45ba5a931855f06e30405a60229938ca
      PATCH_COMMAND ${SOFA_PATCH_COMMAND}
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    ExternalProject_Add_Step(${proj} preconfigure
      COMMAND ${SOFA_PRECONFIGURE_COMMAND}
      WORKING_DIRECTORY <BINARY_DIR>
      DEPENDEES patch
      DEPENDERS configure
      LOG 1
    )

    # SOFA does not support "make install" yet
    # set(SOFA_DIR ${ep_prefix}
    ExternalProject_Get_Property(${proj} binary_dir)
    set(SOFA_DIR ${binary_dir})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
