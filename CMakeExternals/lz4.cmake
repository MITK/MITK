#-----------------------------------------------------------------------------
# lz4
#-----------------------------------------------------------------------------

if(MITK_USE_lz4)
  # Sanity checks
  if(DEFINED lz4_DIR AND NOT EXISTS "${lz4_DIR}")
    message(FATAL_ERROR "lz4_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj lz4)
  set(proj_DEPENDENCIES )
  set(lz4_DEPENDS ${proj})

  if(NOT DEFINED lz4_DIR)

    #[[ lz4 only requires CMake 3.5, i.e. CMP0091 is unset and the MSVC
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
      GIT_REPOSITORY https://github.com/lz4/lz4.git
      GIT_TAG ebb370ca83af193212df4dcbadcc5d87bc0de2f0 # v1.10.0
      SOURCE_SUBDIR build/cmake
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        -DLZ4_BUILD_CLI:BOOL=OFF
        -DLZ4_BUILD_LEGACY_LZ4C:BOOL=OFF
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(lz4_DIR "${ep_prefix}")
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
