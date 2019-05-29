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

    if(APPLE)
      find_library(GLUT_LIBRARY GLUT)
      # add_library(GLUT SHARED IMPORTED)
      # set_property(TARGET GLUT PROPERTY IMPORTED_LOCATION ${GLUT_LIBRARY})
      mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
      get_filename_component(GLUT_DIR ${GLUT_LIBRARY} PATH)
    else()
      set(patch_cmd ${CMAKE_COMMAND} -Dproj:STRING=${proj} -Dproj_target:STRING=freeglut -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake)

      set(additional_args )
      if(CTEST_USE_LAUNCHERS)
        list(APPEND additional_args
          "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
        )
      endif()

      ExternalProject_Add(${proj}
         LIST_SEPARATOR ${sep}
         URL http://mitk.org/download/thirdparty/freeglut-2.8.1.tar.gz
         URL_MD5 918ffbddcffbac83c218bc52355b6d5a
         PATCH_COMMAND ${patch_cmd}
         CMAKE_GENERATOR ${gen}
         CMAKE_GENERATOR_PLATFORM ${gen_platform}
         CMAKE_ARGS
           ${ep_common_args}
           ${additional_args}
         CMAKE_CACHE_ARGS
           ${ep_common_cache_args}
         CMAKE_CACHE_DEFAULT_ARGS
           ${ep_common_cache_default_args}
         DEPENDS ${proj_DEPENDENCIES}
        )

      set(GLUT_DIR ${ep_prefix})
      mitkFunctionInstallExternalCMakeProject(${proj})

    endif()

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
