#-----------------------------------------------------------------------------
# DCMQI
#-----------------------------------------------------------------------------

if(MITK_USE_DCMQI)

  # Sanity checks
  if(DEFINED DCMQI_DIR AND NOT EXISTS ${DCMQI_DIR})
    message(FATAL_ERROR "DCMQI_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj DCMQI)
  set(proj_DEPENDENCIES DCMTK ITK)
  set(DCMQI_DEPENDS ${proj})

  if(NOT DEFINED DCMQI_DIR)
    set(additional_cmake_args)

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      GIT_REPOSITORY https://github.com/nolden/dcmqi.git
      GIT_TAG d067f81c8a8e43900b91861f771013406868672f
      UPDATE_COMMAND ""
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
        #-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DBUILD_SHARED_LIBS:BOOL=ON
        -DDCMQI_BUILD_APPS:BOOL=OFF
        -DDCMTK_DIR:PATH=${DCMTK_DIR}
        -DITK_DIR:PATH=${ITK_DIR}
        -DITK_NO_IO_FACTORY_REGISTER_MANAGER:BOOL=ON
        -DDCMQI_SUPERBUILD:BOOL=OFF
        -DDCMQI_CMAKE_CXX_STANDARD:STRING=14
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )


    ExternalProject_Get_Property(${proj} binary_dir)
    set(DCMQI_DIR ${binary_dir})
    #set(${proj}_DIR ${ep_prefix})
    #message(${proj}_DIR: ${${proj}_DIR})
    #mitkFunctionInstallExternalCMakeProject(${proj})
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
