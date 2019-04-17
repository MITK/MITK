#--------------------------------------------------------------------------
#  PCRE (Perl Compatible Regular Expressions)
#--------------------------------------------------------------------------
if(MITK_USE_PCRE)
  if(DEFINED PCRE_DIR AND NOT EXISTS ${PCRE_DIR})
      message(FATAL_ERROR "PCRE_DIR variable is defined but corresponds to non-existing directory")
   endif()

  set(proj PCRE)
  set(${proj}_DEPENDENCIES "")
  set(${proj}_DEPENDS ${proj})

  if(NOT PCRE_DIR)

    if(UNIX)
      # Some other projects (e.g. Swig) require a pcre-config script which is not
      # generated when using the CMake build system.
      set(configure_cmd
        CONFIGURE_COMMAND <SOURCE_DIR>/./configure
        CC=${CMAKE_C_COMPILER}${CMAKE_C_COMPILER_ARG1}
        CFLAGS=-fPIC
        "CXXFLAGS=-fPIC ${MITK_CXX14_FLAG} ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}"
        "LDFLAGS=${CMAKE_LINKER_FLAGS} ${CMAKE_LINKER_FLAGS_RELEASE} ${_install_rpath_linkflag}"
        CXX=${CMAKE_CXX_COMPILER}${CMAKE_CXX_COMPILER_ARG1}
        --prefix=<INSTALL_DIR>
        --disable-shared
        --enable-jit
      )
    else()

      set(additional_cmake_args )
      if(CTEST_USE_LAUNCHERS)
        list(APPEND additional_cmake_args
          "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
        )
      endif()

      set(configure_cmd
        CMAKE_GENERATOR ${gen}
        CMAKE_GENERATOR_PLATFORM ${gen_platform}
        CMAKE_ARGS
          ${ep_common_args}
          ${additional_cmake_args}
         "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} -fPIC"
         -DBUILD_SHARED_LIBS:BOOL=OFF
         -DPCRE_BUILD_PCREGREP:BOOL=OFF
         -DPCRE_BUILD_TESTS:BOOL=OFF
         -DPCRE_SUPPORT_JIT:BOOL=ON
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
      )
    endif()

    ExternalProject_add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/pcre-8.35.tar.gz
      URL_MD5 "ed58bcbe54d3b1d59e9f5415ef45ce1c"
      ${configure_cmd}
      DEPENDS "${${proj}_DEPENDENCIES}"
      )

    set(PCRE_DIR ${ep_prefix})

  else()
    mitkMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
  endif()
endif()
