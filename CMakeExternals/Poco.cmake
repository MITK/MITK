#-----------------------------------------------------------------------------
# Poco
#-----------------------------------------------------------------------------

if(MITK_USE_Poco)

  # Sanity checks
  if(DEFINED Poco_DIR AND NOT EXISTS ${Poco_DIR})
    message(FATAL_ERROR "Poco_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Poco)
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  if(NOT DEFINED ${proj}_DIR)

    set(additional_cmake_args )
    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/poco-1.9.0.tar.gz
      URL_MD5 1011839033f72de138f0c523c2caa121
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
        -DENABLE_XML:BOOL=ON
        -DENABLE_JSON:BOOL=ON
        -DENABLE_MONGODB:BOOL=OFF
        -DENABLE_PDF:BOOL=OFF
        -DENABLE_UTIL:BOOL=ON
        -DENABLE_NET:BOOL=ON
        -DENABLE_NETSSL:BOOL=OFF
        -DENABLE_NETSSL_WIN:BOOL=OFF
        -DENABLE_CRYPTO:BOOL=OFF
        -DENABLE_DATA:BOOL=OFF
        -DENABLE_DATA_SQLITE:BOOL=OFF
        -DENABLE_DATA_MYSQL:BOOL=OFF
        -DENABLE_DATA_ODBC:BOOL=OFF
        -DENABLE_SEVENZIP:BOOL=OFF
        -DENABLE_ZIP:BOOL=ON
        -DENABLE_APACHECONNECTOR:BOOL=OFF
        -DENABLE_CPPPARSER:BOOL=OFF
        -DENABLE_POCODOC:BOOL=OFF
        -DENABLE_PAGECOMPILER:BOOL=OFF
        -DENABLE_PAGECOMPILER_FILE2PAGE:BOOL=OFF
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
     )

    set(${proj}_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
