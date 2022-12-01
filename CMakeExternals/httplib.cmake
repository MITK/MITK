set(proj httplib)
set(proj_DEPENDENCIES ZLIB)

if(MITK_USE_${proj})
  set(${proj}_DEPENDS ${proj})

  if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory!")
  endif()

  if(NOT DEFINED ${proj}_DIR)
    set(cmake_cache_args
      ${ep_common_cache_args}
      -DHTTPLIB_REQUIRE_OPENSSL:BOOL=ON
      -DHTTPLIB_REQUIRE_ZLIB:BOOL=ON
      -DHTTPLIB_USE_BROTLI_IF_AVAILABLE:BOOL=OFF
    )

    if(OPENSSL_ROOT_DIR)
      list(APPEND cmake_cache_args
        -DOPENSSL_ROOT_DIR:PATH=${OPENSSL_ROOT_DIR}
      )
    endif()

    ExternalProject_Add(${proj}
      GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
      GIT_TAG c0b461a3b7b15877b5c8047b409c124aceec8fd9 # v0.11.3
      CMAKE_ARGS ${ep_common_args}
      CMAKE_CACHE_ARGS ${cmake_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(${proj}_DIR ${ep_prefix})
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
