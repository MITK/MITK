#-----------------------------------------------------------------------------
# litehtml
#-----------------------------------------------------------------------------

if(MITK_USE_litehtml)
  # Sanity checks
  if(DEFINED litehtml_DIR AND NOT EXISTS "${litehtml_DIR}")
    message(FATAL_ERROR "litehtml_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj litehtml)
  set(proj_DEPENDENCIES )
  set(litehtml_DEPENDS ${proj})

  if(NOT DEFINED litehtml_DIR)

    #[[ litehtml requires only CMake 3.11, i.e. CMP0091 is unset and the MSVC
        runtime library would resolve to the statically linked /MT default.
        Force the NEW behavior to use the dynamic runtime library, consistent
        with all other dependencies. ]]
    set(additional_args
      -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
    )

    if(NOT CMAKE_DEBUG_POSTFIX)
      list(APPEND additional_args "-DCMAKE_DEBUG_POSTFIX:STRING=d")
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      GIT_REPOSITORY https://github.com/litehtml/litehtml.git
      GIT_TAG 9bc84b8b8d15a4e50f18b327aa30955048b441c2 # v0.10
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        # Build the bundled gumbo parser (no system dependency) and link
        # everything statically into the MitkQtHtml module. That module is a
        # shared library, so the static archive must be position independent;
        # a static build, unlike a shared one, does not get -fPIC by default.
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
        -DEXTERNAL_GUMBO:BOOL=OFF
        -DLITEHTML_BUILD_TESTING:BOOL=OFF
        # LITEHTML_ENABLE_LINT turns clang-tidy findings into build errors when
        # clang-tidy is on PATH, which breaks otherwise fine builds.
        -DLITEHTML_ENABLE_LINT:BOOL=OFF
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(litehtml_DIR "${ep_prefix}")
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
