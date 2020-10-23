#[============================================================================[

  Configuration

 ]============================================================================]

set(CTEST_CMAKE_GENERATOR
# "Unix Makefiles"
  "Visual Studio 16 2019"
)

set(CTEST_CMAKE_GENERATOR_PLATFORM
# "Win32"
  "x64"
)

set(CTEST_BUILD_CONFIGURATION
# "Debug"
  "Release"
# "MinSizeRel"
# "RelWithDebInfo"
)

set(CTEST_DASHBOARD_MODEL
# "Continuous"
  "Experimental"
# "Nightly"
)

# If empty: Default site name is the name of this computer
set(CTEST_SITE
  ""
)

# If empty: Default build name is automatically generated based on OS,
# compiler, CTEST_BUILD_CONFIGURATION, MITK_BUILD_CONFIGURATION,
# and MITK_BUILD_NAME_SUFFIX
set(CTEST_BUILD_NAME
  ""
)

set(CTEST_BUILD_FLAGS
# "-j8"
)

# Only appended to automatically generated CTEST_BUILD_NAME
set(MITK_BUILD_NAME_SUFFIX
  ""
)

set(MITK_CLEAN_SOURCE_DIR
  OFF
)

set(MITK_CLEAN_BINARY_DIR
  OFF
)

# If empty: Default URL is CTEST_SUBMIT_URL (CTestConfig.cmake)
set(MITK_SUBMIT_URL
  "https://cdash.mitk.org/submit.php?project=MITK"
)

set(MITK_AUTH_TOKEN
  ""
)

set(MITK_BUILD_CONFIGURATION
# "All"
  "WorkbenchRelease"
# "Minimal"
)

set(MITK_BUILD_DOCUMENTATION
  OFF
)

set(MITK_SUPERBUILD_OPTIONS
# "MITK_WHITELIST:STRING=Minimal"
  "Qt5_DIR:PATH=C:/Qt/5.12.9/msvc2017_64/lib/cmake/Qt5"
  "MITK_AUTOCLEAN_EXTERNAL_PROJECTS:BOOL=ON"
  "MITK_FAST_TESTING:BOOL=ON"
  "MITK_XVFB_TESTING:BOOL=OFF"
)

set(MITK_BUILD_OPTIONS
  ""
)

set(MITK_REPOSITORY
  "https://phabricator.mitk.org/source/mitk.git"
)

# Branch name, tag, or commit hash
set(MITK_BRANCH
  "develop"
# "master"
)

# Three list items per MITK extension: <name> <URL> <branch>
set(MITK_EXTENSIONS
# "MITK-ProjectTemplate"
#   "https://phabricator.mitk.org/source/mitk-projecttemplate.git"
#   "master"
# "MITK Diffusion"
#   "https://phabricator.mitk.org/source/mitk-diffusion.git"
#   "master"
)

#[============================================================================[

  Helper functions

 ]============================================================================]

function(get_temp_directory)
  set(temp_directory "${CMAKE_CURRENT_SOURCE_DIR}/temp")

  if(NOT EXISTS "${temp_directory}")
    file(MAKE_DIRECTORY "${temp_directory}")
  elseif(NOT IS_DIRECTORY "${temp_directory}")
    message(FATAL_ERROR "Temporary directory location blocked by file \"${temp_directory}\"")
  endif()

  set(${ARGV0} "${temp_directory}" PARENT_SCOPE)
endfunction()

function(download_file)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "QUIET" "URL;OUTPUT_FILE;ERROR_CODE" "")

  get_filename_component(output_dir "${ARG_OUTPUT_FILE}" DIRECTORY)
  get_filename_component(output_filename "${ARG_OUTPUT_FILE}" NAME)

  if(NOT output_dir)
    set(output_dir "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  set(output_file "${output_dir}/${output_filename}")

  if(NOT ARG_QUIET)
    message("Download file\n  from: ${ARG_URL}\n  to: ${output_file}")
  endif()

  get_temp_directory(temp_dir)
  string(RANDOM random_filename)

  while(EXISTS "${temp_dir}/${random_filename}")
    string(RANDOM random_filename)
  endwhile()

  set(temp_output_file "${temp_dir}/${random_filename}")

  file(DOWNLOAD "${ARG_URL}" "${temp_output_file}" STATUS status)

  list(GET status 0 error_code)
  list(GET status 1 error_message)
  set(error_message "${error_message} (${error_code})")

  if(NOT 0 EQUAL error_code)
    if(EXISTS "${temp_output_file}")
      file(REMOVE "${temp_output_file}")
    endif()

    if(NOT ARG_ERROR_CODE)
      message(FATAL_ERROR "${error_message}")
    elseif(NOT ARG_QUIET)
      message("  error: ${error_message}")
    endif()
  else()
    file(RENAME "${temp_output_file}" "${output_file}")
  endif()

  if(ARG_ERROR_CODE)
    set(${ARG_ERROR_CODE} ${error_code} PARENT_SCOPE)
  endif()
endfunction()

#[============================================================================[

  Download and include actual script

 ]============================================================================]

include("MITKDashboardScript.local.cmake" OPTIONAL RESULT_VARIABLE found)

if(NOT found)
  get_temp_directory(temp_dir)
  set(script "${temp_dir}/MITKDashboardScript.cmake")

  download_file(
    URL "https://raw.githubusercontent.com/MITK/MITK/${MITK_BRANCH}/CMake/MITKDashboardScript.download.cmake"
    OUTPUT_FILE "${script}"
  )

  include("${script}")
endif()
