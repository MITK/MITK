#-----------------------------------------------------------------------------
# freeglut
#-----------------------------------------------------------------------------

if(MITK_USE_GLUT)
  # Sanity checks
  if(DEFINED GLUT_DIR AND NOT EXISTS ${GLUT_DIR})
    message(FATAL_ERROR "GLUT_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj GLUT)
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  if(NOT DEFINED GLUT_DIR)

    set(patch_cmd ${CMAKE_COMMAND} -Dproj:STRING=${proj} -Dproj_target:STRING=GLUT -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake)

    ExternalProject_Add(${proj}
       SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
       BINARY_DIR ${proj}-build
       PREFIX ${proj}-cmake
       URL http://mitk.org/download/thirdparty/freeglut-2.8.1.tar.gz
       URL_MD5 918ffbddcffbac83c218bc52355b6d5a
       PATCH_COMMAND ${patch_cmd}
       INSTALL_COMMAND ""
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(GLUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
