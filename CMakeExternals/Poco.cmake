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

    mitk_query_custom_ep_vars()

    set(ssl_args
      -DENABLE_CRYPTO:BOOL=OFF
      -DENABLE_NETSSL:BOOL=OFF
      -DENABLE_NETSSL_WIN:BOOL=OFF
    )

    if(OpenSSL_FOUND)
      set(ssl_args
        -DENABLE_CRYPTO:BOOL=ON
        -DENABLE_NETSSL:BOOL=ON
        -DFORCE_OPENSSL:BOOL=ON
      )

      if(OPENSSL_ROOT_DIR)
        list(APPEND ssl_args
          "-DOPENSSL_ROOT_DIR:PATH=${OPENSSL_ROOT_DIR}"
        )
      endif()
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      GIT_REPOSITORY https://github.com/pocoproject/poco.git
      GIT_TAG poco-1.12.2-release
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
        ${ssl_args}
        -DENABLE_ACTIVERECORD:BOOL=OFF
        -DENABLE_ACTIVERECORD_COMPILER:BOOL=OFF
        -DENABLE_APACHECONNECTOR:BOOL=OFF
        -DENABLE_CPPPARSER:BOOL=OFF
        -DENABLE_DATA:BOOL=OFF
        -DENABLE_DATA_MYSQL:BOOL=OFF
        -DENABLE_DATA_ODBC:BOOL=OFF
        -DENABLE_DATA_POSTGRESQL:BOOL=OFF
        -DENABLE_DATA_SQLITE:BOOL=OFF
        -DENABLE_ENCODINGS:BOOL=OFF
        -DENABLE_ENCODINGS_COMPILER:BOOL=OFF
        -DENABLE_FOUNDATION:BOOL=ON
        -DENABLE_JSON:BOOL=ON
        -DENABLE_JWT:BOOL=OFF
        -DENABLE_MONGODB:BOOL=OFF
        -DENABLE_NET:BOOL=ON
        -DENABLE_PAGECOMPILER:BOOL=OFF
        -DENABLE_PAGECOMPILER_FILE2PAGE:BOOL=OFF
        -DENABLE_PDF:BOOL=OFF
        -DENABLE_POCODOC:BOOL=OFF
        -DENABLE_PROMETHEUS:BOOL=OFF
        -DENABLE_REDIS:BOOL=OFF
        -DENABLE_SEVENZIP:BOOL=OFF
        -DENABLE_TESTS:BOOL=OFF
        -DENABLE_UTIL:BOOL=ON
        -DENABLE_XML:BOOL=ON
        -DENABLE_ZIP:BOOL=ON
        ${${proj}_CUSTOM_CMAKE_ARGS}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        ${${proj}_CUSTOM_CMAKE_CACHE_ARGS}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
        ${${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS}
      DEPENDS ${proj_DEPENDENCIES}
     )

    set(${proj}_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
