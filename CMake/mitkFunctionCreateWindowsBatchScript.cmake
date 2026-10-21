function(_mitk_get_build_configs var)
  get_property(_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(_multi_config)
    set(${var} ${CMAKE_CONFIGURATION_TYPES} PARENT_SCOPE)
  else()
    set(${var} ${CMAKE_BUILD_TYPE} PARENT_SCOPE)
  endif()
endfunction()

#! Lower-case names of the build configurations of this build tree, the
#! suffixes of the Windows batch scripts. Multi-config generators build
#! CMAKE_CONFIGURATION_TYPES, single-config generators CMAKE_BUILD_TYPE.
#!
#!   mitkFunctionGetWindowsBatchScriptBuildTypes(<var>)
#!
function(mitkFunctionGetWindowsBatchScriptBuildTypes var)
  _mitk_get_build_configs(_configs)
  string(TOLOWER "${_configs}" _build_types)
  set(${var} ${_build_types} PARENT_SCOPE)
endfunction()

function(_mitk_get_windows_runtime_path var build_type)
  get_property(_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(_multi_config)
    set(_intermediate_dir "${build_type}")
  else()
    set(_intermediate_dir "")
  endif()
  string(TOUPPER "${build_type}" _config)
  mitkFunctionGetLibrarySearchPaths(_runtime_path "${_intermediate_dir}" ${_config})
  set(${var} "${_runtime_path}" PARENT_SCOPE)
endfunction()

function(_mitk_create_windows_env_scripts)
  get_property(_dir GLOBAL PROPERTY MITK_WINDOWS_ENV_SCRIPT_DIR)
  file(TO_NATIVE_PATH "${CMAKE_CTEST_COMMAND}" MITK_CTEST_COMMAND)
  file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}" MITK_CTEST_DIR)
  _mitk_get_build_configs(_configs)
  foreach(MITK_BUILD_CONFIG IN LISTS _configs)
    string(TOLOWER "${MITK_BUILD_CONFIG}" MITK_ENV_BUILD_TYPE)
    _mitk_get_windows_runtime_path(MITK_RUNTIME_PATH ${MITK_ENV_BUILD_TYPE})
    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/mitk_env.bat.in"
                   "${_dir}/mitk_env_${MITK_ENV_BUILD_TYPE}.bat" @ONLY)
    if(BUILD_TESTING)
      file(TO_NATIVE_PATH "${_dir}/mitk_env_${MITK_ENV_BUILD_TYPE}.bat" MITK_ENV_SCRIPT)
      configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/mitk_ctest.bat.in"
                     "${_dir}/mitk_ctest_${MITK_ENV_BUILD_TYPE}.bat" @ONLY)
    endif()
  endforeach()
endfunction()

#! Creates mitk_env_<build type>.bat for every build configuration, next to
#! the executables. The scripts run a command with the DLL search paths of
#! the build tree, or open a command prompt with them. With BUILD_TESTING,
#! mitk_ctest_<build type>.bat additionally runs ctest for the build tree with
#! these paths and the configuration set, from any working directory. Calling
#! this function more than once is harmless.
#!
#!   mitkFunctionCreateWindowsEnvScripts()
#!
function(mitkFunctionCreateWindowsEnvScripts)
  get_property(_scheduled GLOBAL PROPERTY MITK_WINDOWS_ENV_SCRIPT_DIR SET)
  if(_scheduled)
    return()
  endif()

  if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  else()
    set(_dir "${CMAKE_BINARY_DIR}")
  endif()
  set_property(GLOBAL PROPERTY MITK_WINDOWS_ENV_SCRIPT_DIR "${_dir}")

  # Modules keep adding search paths while they are configured, so the
  # scripts are written at the very end.
  cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}"
    CALL _mitk_create_windows_env_scripts)
endfunction()

#! Configures the batch script <out> from the template <in> for the build
#! type <build_type>, one of mitkFunctionGetWindowsBatchScriptBuildTypes().
#! The template can use
#!
#!   MITK_ENV_SCRIPT    the mitk_env_<build type>.bat to run the executable with
#!   MITK_EXEC_SUBDIR   the configuration subdirectory of the executable with a
#!                      trailing backslash, empty for single-config generators
#!   MITK_RUNTIME_PATH  the DLL search paths
#!   VS_BUILD_TYPE      <build_type>
#!
function(mitkFunctionCreateWindowsBatchScript in out build_type)
  mitkFunctionCreateWindowsEnvScripts()
  get_property(_env_dir GLOBAL PROPERTY MITK_WINDOWS_ENV_SCRIPT_DIR)
  file(TO_NATIVE_PATH "${_env_dir}/mitk_env_${build_type}.bat" MITK_ENV_SCRIPT)

  get_property(_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(_multi_config)
    set(MITK_EXEC_SUBDIR "${build_type}\\")
  else()
    set(MITK_EXEC_SUBDIR "")
  endif()

  _mitk_get_windows_runtime_path(MITK_RUNTIME_PATH ${build_type})
  set(VS_BUILD_TYPE ${build_type})

  configure_file(${in} ${out} @ONLY)
endfunction()
