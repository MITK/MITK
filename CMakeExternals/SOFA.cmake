#-----------------------------------------------------------------------------
# SOFA
#-----------------------------------------------------------------------------

if(MITK_USE_SOFA)
  # Sanity checks
  if(DEFINED SOFA_DIR AND NOT EXISTS ${SOFA_DIR})
    message(FATAL_ERROR "SOFA_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj SOFA)
  set(proj_DEPENDENCIES)
  set(SOFA_DEPENDS ${proj})

  set(additional_cmake_args
    -DSOFA-APPLICATION_MODELER:BOOL=OFF
    -DSOFA-APPLICATION_RUN_SOFA:BOOL=OFF
    -DSOFA-APPLICATION_SOFA_BATCH:BOOL=OFF
    -DSOFA-EXTERNAL_HAVE_ZLIB:BOOL=OFF
    -DSOFA-EXTERNAL_HAVE_PNG:BOOL=OFF
    -DSOFA-LIB_COMPONENT_EIGEN2_SOLVER:BOOL=ON
    -DSOFA-LIB_COMPONENT_OPENGL_VISUAL:BOOL=OFF
    -DSOFA-LIB_GUI_GLUT:BOOL=OFF
    -DSOFA-LIB_GUI_QTVIEWER:BOOL=OFF
    -DSOFA-MISC_NO_OPENGL:BOOL=ON
    -DSOFA-TUTORIAL_CHAIN_HYBRID:BOOL=OFF
    -DSOFA-TUTORIAL_COMPOSITE_OBJECT:BOOL=OFF
    -DSOFA-TUTORIAL_HOUSE_OF_CARDS:BOOL=OFF
    -DSOFA-TUTORIAL_MIXED_PENDULUM:BOOL=OFF
    -DSOFA-TUTORIAL_ONE_PARTICLE:BOOL=OFF
    -DSOFA-TUTORIAL_ONE_TETRAHEDRON:BOOL=OFF)

  set(SOFA_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchSOFA-rev9479.cmake)
  set(SOFA_PRECONFIGURE_COMMAND ${CMAKE_COMMAND} -G${gen} ${CMAKE_BINARY_DIR}/${proj}-src)
  set(SOFA_CONFIGURE_COMMAND ${CMAKE_COMMAND} -G${gen} ${ep_common_args} ${additional_cmake_args} ${CMAKE_BINARY_DIR}/${proj}-src)

  if(NOT DEFINED SOFA_DIR)
    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      URL http://mitk.org/download/thirdparty/SOFA-rev9479.tar.gz
      URL_MD5 fa43e6183abb8fc36c86ec43a095fdc2
      PATCH_COMMAND ${SOFA_PATCH_COMMAND}
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
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
    mitkMacroEmptyExternalProject(${proj} "${proj}_DEPENDENCIES}")
  endif()
endif()
