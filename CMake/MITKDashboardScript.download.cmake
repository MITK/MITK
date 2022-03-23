#[============================================================================[

  Helper functions

 ]============================================================================]

function(get_macos_codename)
  set(macos_codename "")

  set(software_license_file "\
/System/Library/CoreServices/Setup Assistant.app\
/Contents/Resources/en.lproj/OSXSoftwareLicense.rtf")

  if(EXISTS "${software_license_file}")
    file(READ "${software_license_file}" software_license)

    if(software_license MATCHES "SOFTWARE LICENSE AGREEMENT FOR macOS ([^\\\n]+)")
      set(macos_codename "${CMAKE_MATCH_1}")
    endif()
  endif()

  set(${ARGV0} ${macos_codename} PARENT_SCOPE)
endfunction()

function(get_macos_name)
  set(macos_name "macOS")

  get_macos_codename(macos_codename)

  if(macos_codename)
    set(macos_name "${macos_name} ${macos_codename}")
  endif()

  execute_process(
    RESULT_VARIABLE error_code
    OUTPUT_VARIABLE version
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND "sw_vers" "-productVersion")

  if(0 EQUAL error_code AND version)
    set(macos_name "${macos_name} ${version}")
  endif()

  set(${ARGV0} ${macos_name} PARENT_SCOPE)
endfunction()

function(get_linux_name)
  set(linux_name "Linux")
  set(os_release_file "/etc/os-release")

  if(EXISTS "${os_release_file}")
    file(STRINGS "${os_release_file}" os_release)

    set(name "")
    set(version_id "")

    foreach(line ${os_release})
      if(NOT name AND line MATCHES "^NAME=[\"]?([^\"]+)")
        set(name "${CMAKE_MATCH_1}")
        continue()
      endif()

      if(NOT version_id AND line MATCHES "^VERSION_ID=\"?([^\"]+)")
        set(version_id "${CMAKE_MATCH_1}")
        continue()
      endif()

      if(name AND version_id)
        break()
      endif()
    endforeach()

    if(name)
      set(linux_name "${name}")
    endif()

    if(version_id)
      set(linux_name "${linux_name} ${version_id}")
    endif()
  endif()

  set(${ARGV0} ${linux_name} PARENT_SCOPE)
endfunction()

function(get_windows_name)
  set(windows_name "Windows")

  execute_process(
    RESULT_VARIABLE error_code
    OUTPUT_VARIABLE version
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND "wmic" "os" "get" "Version" "-value")

  if(0 EQUAL error_code AND version)
    if(version MATCHES "Version=([0-9]+)\\.([0-9]+)")
      set(windows_name "${windows_name} ${CMAKE_MATCH_1}")
      if(NOT 0 EQUAL CMAKE_MATCH_2)
        set(windows_name "${windows_name}.${CMAKE_MATCH_2}")
      endif()
    endif()
  endif()

  set(${ARGV0} ${windows_name} PARENT_SCOPE)
endfunction()

function(get_os_name)
  set(os_name "")

  if(APPLE)
    get_macos_name(os_name)
  elseif(UNIX)
    get_linux_name(os_name)
  elseif(WIN32)
    get_windows_name(os_name)
  endif()

  set(${ARGV0} ${os_name} PARENT_SCOPE)
endfunction()

function(download_string)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "URL;OUTPUT_VARIABLE" "")

  get_temp_directory(temp_dir)
  string(RANDOM random_filename)

  while(EXISTS "${temp_dir}/${random_filename}")
    string(RANDOM random_filename)
  endwhile()

  set(temp_output_file "${temp_dir}/${random_filename}")

  download_file(
    URL ${ARG_URL}
    OUTPUT_FILE ${temp_output_file}
    ERROR_CODE error_code)

  set(output "")

  if(EXISTS "${temp_output_file}")
    file(READ "${temp_output_file}" output)
    file(REMOVE "${temp_output_file}")
  endif()

  set(${ARG_OUTPUT_VARIABLE} ${output} PARENT_SCOPE)
endfunction()

function(find_vswhere)
  get_temp_directory(temp_dir)
  set(latest_vswhere "${temp_dir}/vswhere.exe")
  set(vswhere "vswhere.exe-NOTFOUND")

  if(EXISTS "${latest_vswhere}")
    set(vswhere "${latest_vswhere}")
  else()
    set(program_files_x86 "PROGRAMFILES(X86)")
    set(program_files_x86 "$ENV{${program_files_x86}}")
    set(system_vswhere "${program_files_x86}/Microsoft Visual Studio/Installer/vswhere.exe")

    if(EXISTS "${system_vswhere}")
      set(vswhere "${system_vswhere}")
    else()
      download_string(
        URL "https://api.github.com/repos/microsoft/vswhere/releases/latest"
        OUTPUT_VARIABLE latest_vswhere_json)

      if(latest_vswhere_json MATCHES "\"browser_download_url\": *\"([^\"]*)\"")
        download_file(
          URL "${CMAKE_MATCH_1}"
          OUTPUT_FILE "${latest_vswhere}"
          ERROR_CODE error_code)

        if (0 EQUAL error_code)
          set(vswhere "${latest_vswhere}")
        endif()
      endif()
    endif()
  endif()

  set(${ARGV0} "${vswhere}" PARENT_SCOPE)
endfunction()

function(vswhere)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "PROPERTY;OUTPUT_VARIABLE" "")

  set(property_value "")

  if(CTEST_CMAKE_GENERATOR MATCHES "^Visual Studio ([0-9]+)")
    set(major_version "${CMAKE_MATCH_1}")
    math(EXPR next_major_version "${major_version} + 1")

    find_vswhere(vswhere)

    if (vswhere)
      execute_process(
        OUTPUT_VARIABLE property_value
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND ${vswhere}
          "-requires" "Microsoft.VisualStudio.Workload.NativeDesktop"
          "-version" "[${major_version},${next_major_version})"
          "-property" "${ARG_PROPERTY}"
          "-format" "value")
    endif()
  endif()

  set(${ARG_OUTPUT_VARIABLE} ${property_value} PARENT_SCOPE)
endfunction()

function(get_visual_studio_name)
  set(visual_studio_name "Visual Studio")

  if(CTEST_CMAKE_GENERATOR MATCHES "^Visual Studio [0-9]+ ([0-9]+)")
    set(product_line_version "${CMAKE_MATCH_1}")
    set(visual_studio_name "${visual_studio_name} ${product_line_version}")

    vswhere(PROPERTY "catalog_productDisplayVersion" OUTPUT_VARIABLE product_display_version)

    if(product_display_version)
      set(visual_studio_name "${visual_studio_name} ${product_display_version}")
    endif()
  endif()

  set(${ARGV0} ${visual_studio_name} PARENT_SCOPE)
endfunction()

function(get_compiler_name)
  set(compiler_name "")

  if(CTEST_CMAKE_GENERATOR MATCHES "^Visual Studio")
    get_visual_studio_name(compiler_name)
  else()
    get_temp_directory(temp_dir)

    file(WRITE "${temp_dir}/src/CMakeLists.txt" "\
project(probe C)
message(STATUS \"CMAKE_C_COMPILER_ID=\\\"\${CMAKE_C_COMPILER_ID}\\\"\")
message(STATUS \"CMAKE_C_COMPILER_VERSION=\\\"\${CMAKE_C_COMPILER_VERSION}\\\"\")")

    file(REMOVE_RECURSE "${temp_dir}/build")

    unset(options)

    foreach(option ${MITK_SUPERBUILD_OPTIONS})
      list(APPEND options "-D" "${option}")
    endforeach()

    execute_process(
      RESULT_VARIABLE error_code
      OUTPUT_VARIABLE compiler
      ERROR_QUIET
      COMMAND ${CMAKE_COMMAND}
        "-S" "${temp_dir}/src"
        "-B" "${temp_dir}/build"
        "-G" "${CTEST_CMAKE_GENERATOR}"
        ${options})

    if(0 EQUAL error_code)
      if(compiler MATCHES "CMAKE_C_COMPILER_ID=\"([^\"]*)\"")
        set(compiler_name "${CMAKE_MATCH_1}")

        if(compiler MATCHES "CMAKE_C_COMPILER_VERSION=\"([^\"]*)\"")
          set(version "${CMAKE_MATCH_1}")

          if(version MATCHES "^([0-9]+\\.[0-9]+\\.[0-9]+)")
            set(version "${CMAKE_MATCH_1}")
          endif()

          string(REGEX REPLACE "\\.0$" "" version "${version}")

          set(compiler_name "${compiler_name} ${version}")
        endif()
      endif()
    endif()
  endif()

  set(${ARGV0} ${compiler_name} PARENT_SCOPE)
endfunction()

function(get_default_build_name)
  get_os_name(default_build_name)
  get_compiler_name(compiler_name)

  if(compiler_name)
    set(default_build_name "${default_build_name} ${compiler_name}")
  endif()

  set(default_build_name "${default_build_name} ${CTEST_BUILD_CONFIGURATION}")

  if(MITK_BUILD_CONFIGURATION)
    set(default_build_name "${default_build_name} ${MITK_BUILD_CONFIGURATION}")
  endif()

  if(MITK_BUILD_NAME_SUFFIX)
    set(default_build_name "${default_build_name} ${MITK_BUILD_NAME_SUFFIX}")
  endif()

  set(${ARGV0} ${default_build_name} PARENT_SCOPE)
endfunction()

function(git)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "WORKING_DIRECTORY;OUTPUT_VARIABLE;RESULT_VARIABLE" "")

  if(NOT ARG_WORKING_DIRECTORY)
    set(ARG_WORKING_DIRECTORY "${CTEST_SOURCE_DIRECTORY}")
  endif()

  get_filename_component(ARG_WORKING_DIRECTORY "${ARG_WORKING_DIRECTORY}" ABSOLUTE)

  execute_process(
    WORKING_DIRECTORY "${ARG_WORKING_DIRECTORY}"
    RESULT_VARIABLE error_code
    OUTPUT_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND "${CTEST_GIT_COMMAND}" ${ARG_UNPARSED_ARGUMENTS})

  if(ARG_OUTPUT_VARIABLE)
    set(${ARG_OUTPUT_VARIABLE} ${output} PARENT_SCOPE)
  endif()

  if(ARG_RESULT_VARIABLE)
    set(${ARG_RESULT_VARIABLE} ${error_code} PARENT_SCOPE)
  endif()
endfunction()

function(git_log)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "WORKING_DIRECTORY;COMMIT_HASH;COMMITTER;SUBJECT" "")

  if(NOT ARG_WORKING_DIRECTORY)
    set(ARG_WORKING_DIRECTORY "${CTEST_SOURCE_DIRECTORY}")
  endif()

  get_filename_component(ARG_WORKING_DIRECTORY "${ARG_WORKING_DIRECTORY}" ABSOLUTE)

  set(args WORKING_DIRECTORY "${ARG_WORKING_DIRECTORY}" "log" "-1")

  if(ARG_COMMIT_HASH)
    unset(commit_hash)
    git(OUTPUT_VARIABLE commit_hash ${args} "--pretty=%h" )
    if(commit_hash)
      set(${ARG_COMMIT_HASH} ${commit_hash} PARENT_SCOPE)
    endif()
  endif()

  if(ARG_COMMITTER)
    unset(committer)
    git(OUTPUT_VARIABLE committer ${args} "--pretty=%cn" )
    if(committer)
      set(${ARG_COMMITTER} ${committer} PARENT_SCOPE)
    endif()
  endif()

  if(ARG_SUBJECT)
    unset(subject)
    git(OUTPUT_VARIABLE subject ${args} "--pretty=%s" )
    if(subject)
      set(${ARG_SUBJECT} ${subject} PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(rm)
  execute_process(COMMAND ${CMAKE_COMMAND} "-E" "rm" "-rf" ${ARGN})
endfunction()

function(submit)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "" "PARTS")

  if(MITK_SUBMIT_URL)
    set(submit_url "SUBMIT_URL" "${MITK_SUBMIT_URL}")
  else()
    unset(submit_url)
  endif()

  if(MITK_AUTH_TOKEN)
    set(http_header HTTPHEADER "Authorization: Bearer ${MITK_AUTH_TOKEN}")
  else()
    unset(http_header)
  endif()

  ctest_submit(${submit_url} ${http_header} RETRY_COUNT 3 RETRY_DELAY 5 PARTS ${ARG_PARTS})
endfunction()

#[============================================================================[

  Actual script

 ]============================================================================]

cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

find_program(CTEST_GIT_COMMAND git)

if(NOT CTEST_GIT_COMMAND)
  message(FATAL_ERROR "Git not found")
endif()

if(NOT CTEST_CMAKE_GENERATOR MATCHES "^Visual Studio")
  unset(CTEST_CMAKE_GENERATOR_PLATFORM)
endif()

if(CTEST_CMAKE_GENERATOR MATCHES "^(Unix Makefiles|Ninja)$")
  set(CTEST_USE_LAUNCHERS ON)
endif()

if(NOT CTEST_SITE)
  unset(CTEST_SITE)
  site_name(CTEST_SITE)
endif()

if(NOT CTEST_BUILD_NAME)
  get_default_build_name(CTEST_BUILD_NAME)
endif()

set(CTEST_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/MITK")
set(CTEST_BINARY_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

set(indent "   ")

message("CTest settings")

if(MITK_SUBMIT_URL)
  message("${indent}Submit URL: ${MITK_SUBMIT_URL}")
endif()

message("\
${indent}Dashboard model: ${CTEST_DASHBOARD_MODEL}
${indent}Build name: ${CTEST_BUILD_NAME}
${indent}Site: ${CTEST_SITE}")

if(MITK_CLEAN_SOURCE_DIR OR MITK_CLEAN_BINARY_DIR)
  message("Clean")

  if(MITK_CLEAN_SOURCE_DIR)
     set(clean_dir "${CMAKE_CURRENT_SOURCE_DIR}/src")
     message("${indent}Source directory: ${clean_dir}")
     rm("${clean_dir}")
  endif()

  if(MITK_CLEAN_BINARY_DIR)
    set(clean_dir "${CMAKE_CURRENT_SOURCE_DIR}/build")
    message("${indent}Binary directory: ${clean_dir}")
    rm("${clean_dir}")
  endif()
endif()

message("MITK repository")

if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}")
  message("\
${indent}Clone repository: ${MITK_REPOSITORY}
${indent}Branch: ${MITK_BRANCH}")

  set(CTEST_CHECKOUT_COMMAND "\"${CTEST_GIT_COMMAND}\" clone --branch=${MITK_BRANCH} -- ${MITK_REPOSITORY} MITK")

  ctest_start(${CTEST_DASHBOARD_MODEL})
else()
  git_log(COMMIT_HASH old_revision COMMITTER committer SUBJECT subject)

  message("\
${indent}Update repository: ${CTEST_SOURCE_DIRECTORY}
${indent}Branch: ${MITK_BRANCH}
${indent}Old revision: ${old_revision} (committed by ${committer})
${indent}${indent}${subject}")

  ctest_start(${CTEST_DASHBOARD_MODEL})

  set(CTEST_UPDATE_OPTIONS "--tags origin ${MITK_BRANCH}")

  ctest_update(RETURN_VALUE return_value)

  if(-1 EQUAL return_value)
    return()
  endif()

  git_log(COMMIT_HASH new_revision)

  if(new_revision STREQUAL old_revision)
    message("${indent}New revision: up-to-date")
  else()
    git_log(COMMITTER committer SUBJECT subject)

    message("\
${indent}New revision: ${new_revision} (committed by ${committer})
${indent}${indent}${subject}")
  endif()
endif()

unset(extension_dirs)

if(MITK_EXTENSIONS)
  list(LENGTH MITK_EXTENSIONS mitk_extensions_size)
  math(EXPR max_index "${mitk_extensions_size} - 1")

  foreach(name_index RANGE 0 ${max_index} 3)
    list(GET MITK_EXTENSIONS ${name_index} name)

    math(EXPR url_index "${name_index} + 1")
    list(GET MITK_EXTENSIONS ${url_index} url)

    math(EXPR branch_index "${name_index} + 2")
    list(GET MITK_EXTENSIONS ${branch_index} branch)

    message("${name} repository")

    set(extension_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/${name}")
    list(APPEND extension_dirs "${extension_dir}")

    if(NOT EXISTS "${extension_dir}")
      message("\
${indent}Clone repository: ${url}
${indent}Branch: ${branch}")

      git("clone" "--quiet" "--branch=${branch}" "${url}" "${name}"
        WORKING_DIRECTORY "${extension_dir}/..")
    else()
      git_log(COMMIT_HASH old_revision COMMITTER committer SUBJECT subject
        WORKING_DIRECTORY "${extension_dir}")

      message("\
${indent}Update repository: ${extension_dir}
${indent}Branch: ${branch}
${indent}Old revision: ${old_revision} (committed by ${committer})
${indent}${indent}${subject}")

      git("fetch" "--quiet" "--tags" "origin" "${branch}"
        WORKING_DIRECTORY "${extension_dir}")

      git("diff" "--quiet" "HEAD" "FETCH_HEAD"
        WORKING_DIRECTORY "${extension_dir}"
        RESULT_VARIABLE error_code)

      if(NOT error_code EQUAL 0)
        git("reset" "--quiet" "--hard" "FETCH_HEAD"
          WORKING_DIRECTORY "${extension_dir}")
      endif()

      git_log(COMMIT_HASH new_revision
        WORKING_DIRECTORY "${extension_dir}")

      if(new_revision STREQUAL old_revision)
        message("${indent}New revision: up-to-date")
      else()
        git_log(COMMITTER committer SUBJECT subject
          WORKING_DIRECTORY "${extension_dir}")

        message("\
${indent}New revision: ${new_revision} (committed by ${committer})
${indent}${indent}${subject}")
      endif()
    endif()
  endforeach()
endif()

submit(PARTS Update)

set(options
  "-D" "MITK_CTEST_SCRIPT_MODE:STRING=${CTEST_DASHBOARD_MODEL}"
# "-D" "SUPERBUILD_EXCLUDE_MITKBUILD_TARGET:BOOL=TRUE"
)

if(NOT WIN32)
  list(APPEND options "-D" "CMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}")
endif()

if(extension_dirs)
  string (REPLACE ";" "\\\;" extension_dirs "${extension_dirs}")
  list(APPEND options "-D" "MITK_EXTENSION_DIRS:STRING=${extension_dirs}")
endif()

if(MITK_BUILD_CONFIGURATION)
  list(APPEND options "-D" "MITK_BUILD_CONFIGURATION:STRING=${MITK_BUILD_CONFIGURATION}")
endif()

if(MITK_BUILD_OPTIONS)
  get_temp_directory(temp_dir)
  set(mitk_initial_cache_file "${temp_dir}/MITKInitialCache.cmake")
  file(WRITE "${mitk_initial_cache_file}" "")
  foreach(mitk_option ${MITK_BUILD_OPTIONS})
    if(mitk_option MATCHES "^([^:]+):([^=]+)=(.*)")
      set(var "${CMAKE_MATCH_1}")
      set(type "${CMAKE_MATCH_2}")
      set(value "${CMAKE_MATCH_3}")
      file(APPEND "${mitk_initial_cache_file}" "set(${var} \"${value}\" CACHE ${type} \"\" FORCE)\n")
    endif()
  endforeach()
  list(APPEND options "-D" "MITK_INITIAL_CACHE_FILE:FILEPATH=${mitk_initial_cache_file}")
endif()

foreach(option ${MITK_SUPERBUILD_OPTIONS})
  list(APPEND options "-D" "${option}")
endforeach()

ctest_configure(OPTIONS "${options}" RETURN_VALUE return_value)
submit(PARTS Configure)

if(NOT 0 EQUAL return_value)
  submit(PARTS Done)
  return()
endif()

include("${CTEST_BINARY_DIRECTORY}/SuperBuildTargets.cmake")

if(NOT SUPERBUILD_TARGETS)
  submit(PARTS Done)
  message(FATAL_ERROR "SUPERBUILD_TARGETS variable not set in SuperBuildTargets.cmake")
else()
  set(mitk_targets
    MITK-Data
    MITK-Utilities
    MITK-Configure
    MITK-build
  )

  list(LENGTH SUPERBUILD_TARGETS n)
  list(LENGTH mitk_targets num_mitk_targets)
  math(EXPR n "${n} + ${num_mitk_targets}")
  set(i 1)

  unset(append)

  foreach(target ${SUPERBUILD_TARGETS} ${mitk_targets})
    message("MITK superbuild - [${i}/${n}] Build ${target}")

    if(NOT target IN_LIST mitk_targets)
      list(APPEND CTEST_CUSTOM_WARNING_EXCEPTION ".*")
      set(pop_warning_exception TRUE)
    else()
      set(pop_warning_exception FALSE)
    endif()

    ctest_build(TARGET ${target}
      NUMBER_ERRORS num_build_errors
      NUMBER_WARNINGS num_build_warnings
      RETURN_VALUE return_value
      ${append})

    if(pop_warning_exception)
      list(POP_BACK CTEST_CUSTOM_WARNING_EXCEPTION)
    endif()

    submit(PARTS Build)

    if(0 LESS num_build_warnings)
      message("${indent}${num_build_warnings} warning(s)")
    endif()

    if(NOT (0 EQUAL return_value AND 0 EQUAL num_build_errors))
      submit(PARTS Done)
      message("${indent}${num_build_errors} error(s)")
      return()
    else()
      message("${indent}${target} was built successfully")
    endif()

    if(NOT append)
      set(append APPEND)
    endif()

    math(EXPR i "${i} + 1")
  endforeach()
endif()

if(MITK_BUILD_DOCUMENTATION)
  message("MITK build - Build documentation")

  list(APPEND CTEST_CUSTOM_WARNING_EXCEPTION ".*")

  ctest_build(TARGET doc
    BUILD "${CTEST_BINARY_DIRECTORY}/MITK-build"
    NUMBER_ERRORS num_doc_errors
    NUMBER_WARNINGS num_doc_warnings
    RETURN_VALUE return_value
    APPEND
  )

  list(POP_BACK CTEST_CUSTOM_WARNING_EXCEPTION)

  submit(PARTS Build)

  if(0 LESS num_doc_warnings)
    message("${indent}${num_doc_warnings} warning(s)")
  endif()

  if(NOT (0 EQUAL return_value AND 0 EQUAL num_doc_errors))
    submit(PARTS Done)
    message("${indent}${num_doc_errors} error(s)")
    return()
  else()
    message("${indent}Documentation was built successfully")
  endif()
endif()

message("Run unit tests...")

set(CTEST_CONFIGURATION_TYPE "${CTEST_BUILD_CONFIGURATION}")

ctest_test(
  BUILD "${CTEST_BINARY_DIRECTORY}/MITK-build"
  PARALLEL_LEVEL 4)

submit(PARTS Test Done)

message("Done")
