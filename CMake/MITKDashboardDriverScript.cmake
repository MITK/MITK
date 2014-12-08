
#
# Included from a dashboard script, this cmake file will drive the configure and build
# steps of MITK
#

#-----------------------------------------------------------------------------
# The following variable are expected to be define in the top-level script:
set(expected_variables
  CTEST_NOTES_FILES
  CTEST_SITE
  CTEST_DASHBOARD_ROOT
  CTEST_CMAKE_COMMAND
  CTEST_CMAKE_GENERATOR
  WITH_MEMCHECK
  WITH_COVERAGE
  WITH_DOCUMENTATION
  CTEST_BUILD_CONFIGURATION
  CTEST_TEST_TIMEOUT
  CTEST_BUILD_FLAGS
  TEST_TO_EXCLUDE_REGEX
  CTEST_SOURCE_DIRECTORY
  CTEST_BINARY_DIRECTORY
  CTEST_BUILD_NAME
  SCRIPT_MODE
  CTEST_COVERAGE_COMMAND
  CTEST_MEMORYCHECK_COMMAND
  CTEST_GIT_COMMAND
  PROJECT_BUILD_DIR
  SUPERBUILD_TARGETS
  )

foreach(var ${expected_variables})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} should be defined in top-level script !")
  endif()
endforeach()

string(REPLACE " " "%20" _build_name_escaped "${CTEST_BUILD_NAME}")

# Check if "mbits" is reachable
file(DOWNLOAD "http://mbits" "${CTEST_SCRIPT_DIRECTORY}/mbits.html" TIMEOUT 2 STATUS _status)
list(GET _status 0 _status_code)
if(_status_code EQUAL 6) # couldn't resovle host name
  set(MBITS_AVAILABLE 0)
else()
  set(MBITS_AVAILABLE 1)
endif()

#
# Site specific options
#
if(NOT CDASH_ADMIN_URL_PREFIX AND MBITS_AVAILABLE)
  set(CDASH_ADMIN_URL_PREFIX "http://mbits")
endif()

if(NOT MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL AND MBITS_AVAILABLE)
  set(MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL "http://mbits/dl-cache")
endif()


if(NOT DEFINED GIT_BRANCH OR GIT_BRANCH STREQUAL "")
  set(GIT_BRANCH "")
else()
  set(GIT_BRANCH "-b ${GIT_BRANCH}")
endif()

# Should binary directory be cleaned?
set(empty_binary_directory FALSE)

# Attempt to build and test also if 'ctest_update' returned an error
set(initial_force_build FALSE)

# Set model options
set(model "")
if(SCRIPT_MODE STREQUAL "experimental")
  set(empty_binary_directory FALSE)
  set(initial_force_build TRUE)
  set(model Experimental)
elseif(SCRIPT_MODE STREQUAL "continuous")
  set(empty_binary_directory FALSE)
  set(initial_force_build FALSE)
  set(model Continuous)
elseif(SCRIPT_MODE STREQUAL "nightly")
  set(empty_binary_directory TRUE)
  set(initial_force_build TRUE)
  set(model Nightly)
else()
  message(FATAL_ERROR "Unknown script mode: '${SCRIPT_MODE}'. Script mode should be either 'experimental', 'continuous' or 'nightly'")
endif()

#message("script_mode:${SCRIPT_MODE}")
#message("model:${model}")
#message("empty_binary_directory:${empty_binary_directory}")
#message("force_build:${initial_force_build}")

set(CTEST_CONFIGURATION_TYPE ${CTEST_BUILD_CONFIGURATION})

if(empty_binary_directory)
  message("Directory ${CTEST_BINARY_DIRECTORY} cleaned !")
  ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
endif()

if(NOT DEFINED CTEST_CHECKOUT_DIR)
  set(CTEST_CHECKOUT_DIR ${CTEST_SOURCE_DIRECTORY})
endif()

if(NOT EXISTS "${CTEST_CHECKOUT_DIR}")
  set(CTEST_CHECKOUT_COMMAND "\"${CTEST_GIT_COMMAND}\" clone ${GIT_BRANCH} ${GIT_REPOSITORY} \"${CTEST_CHECKOUT_DIR}\"")
endif()

set(CTEST_UPDATE_TYPE "git")
set(CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}")

#----------------------------------------------------------------------
# Utility macros
#----------------------------------------------------------------------

function(func_build_target target build_dir)
  set(CTEST_BUILD_TARGET ${target})
  ctest_build(BUILD "${build_dir}" APPEND
              RETURN_VALUE res
              NUMBER_ERRORS num_errors
              NUMBER_WARNINGS num_warnings)
  ctest_submit(PARTS Build RETRY_DELAY 3 RETRY_COUNT 3)

  if(num_errors)
    math(EXPR build_errors "${build_errors} + ${num_errors}")
    set(build_errors ${build_errors} PARENT_SCOPE)
  endif()
  if(num_warnings)
    math(EXPR build_warnings "${build_warnings} + ${num_warnings}")
    set(build_warnings ${build_warnings} PARENT_SCOPE)
  endif()
endfunction()

function(func_test label build_dir)
  if(NOT TESTING_PARALLEL_LEVEL)
    set(TESTING_PARALLEL_LEVEL 8)
  endif()

  if(label MATCHES "Unlabeled")
    set(_include_label EXCLUDE_LABEL .*)
  else()
    set(_include_label INCLUDE_LABEL ${label})
  endif()

  ctest_test(BUILD "${build_dir}"
             ${_include_label}
             PARALLEL_LEVEL ${TESTING_PARALLEL_LEVEL}
             EXCLUDE ${TEST_TO_EXCLUDE_REGEX}
             RETURN_VALUE res
            )
  ctest_submit(PARTS Test RETRY_DELAY 3 RETRY_COUNT 3)

  if(res)
    math(EXPR test_errors "${test_errors} + 1")
    set(test_errors ${test_errors} PARENT_SCOPE)
  endif()

  if(ARG3)
    set(WITH_COVERAGE ${ARG3})
  endif()
  if(ARG4)
    set(WITH_MEMCHECK ${ARG4})
  endif()

  if(WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
    message("----------- [ Coverage ${label} ] -----------")
    ctest_coverage(BUILD "${build_dir}" LABELS ${label})
    ctest_submit(PARTS Coverage RETRY_DELAY 3 RETRY_COUNT 3)
  endif()

  if(WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
    if(NOT CTEST_MEMORYCHECK_SUPPRESSIONS_FILE)
      if(EXISTS "${CTEST_SOURCE_DIRECTORY}/CMake/valgrind.supp")
        set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE "${CTEST_SOURCE_DIRECTORY}/CMake/valgrind.supp")
      endif()
    endif()
    if(NOT CTEST_MEMORYCHECK_COMMAND_OPTIONS)
      set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "-q --tool=memcheck --leak-check=yes --show-reachable=no --show-possibly-lost=no --workaround-gcc296-bugs=yes --num-callers=50")
    endif()
    ctest_memcheck(BUILD "${build_dir}" INCLUDE_LABEL ${label})
    ctest_submit(PARTS MemCheck RETRY_DELAY 3 RETRY_COUNT 3)
  endif()

endfunction()

#---------------------------------------------------------------------
# run_ctest macro
#---------------------------------------------------------------------
macro(run_ctest)

  set(build_warnings 0)
  set(build_errors 0)
  set(test_errors 0)

  ctest_start(${model})

  ctest_update(SOURCE "${CTEST_CHECKOUT_DIR}" RETURN_VALUE res)

  if(res LESS 0)
    # update error
    math(EXPR build_errors "${build_errors} + 1")
  endif()

  set(force_build ${initial_force_build})

  # Check if a forced run was requested
  set(cdash_remove_rerun_url )
  if(CDASH_ADMIN_URL_PREFIX)
    set(cdash_rerun_url "${CDASH_ADMIN_URL_PREFIX}/rerun/${_build_name_escaped}")
    set(cdash_remove_rerun_url "${CDASH_ADMIN_URL_PREFIX}/rerun/rerun.php?name=${_build_name_escaped}&remove=1")
    file(DOWNLOAD
         "${cdash_rerun_url}"
         "${CTEST_BINARY_DIRECTORY}/tmp.txt"
         STATUS status
         )
    list(GET status 0 error_code)
    file(READ "${CTEST_BINARY_DIRECTORY}/tmp.txt" rerun_content LIMIT 1)
    if(NOT error_code AND NOT rerun_content)
      set(force_build 1)
    endif()
  endif()

  if(COMMAND MITK_OVERRIDE_FORCE_BUILD)
    MITK_OVERRIDE_FORCE_BUILD(force_build)
  endif()

  # force a build if this is the first run and the build dir is empty
  if(NOT EXISTS "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt")
    message("First time build - Initialize CMakeCache.txt")
    set(res 1)

    # Write initial cache.
    if(NOT DEFINED BUILD_TESTING)
      set(BUILD_TESTING ON)
    endif()

    # Write initial cache.
    file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CTEST_USE_LAUNCHERS:BOOL=${CTEST_USE_LAUNCHERS}
CTEST_PROJECT_ADDITIONAL_TARGETS:INTERNAL=${CTEST_PROJECT_ADDITIONAL_TARGETS}
BUILD_TESTING:BOOL=${BUILD_TESTING}
MITK_CTEST_SCRIPT_MODE:STRING=${SCRIPT_MODE}
CMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}
WITH_COVERAGE:BOOL=${WITH_COVERAGE}
MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL:STRING=${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}
${INITIAL_CMAKECACHE_OPTIONS}
")
  endif()

  if(res GREATER 0 OR force_build)

    # Clear the forced rerun request
    if(CDASH_ADMIN_URL_PREFIX AND cdash_remove_rerun_url)
      file(DOWNLOAD "${cdash_remove_rerun_url}" "${CTEST_BINARY_DIRECTORY}/tmp.txt")
      file(REMOVE "${CTEST_BINARY_DIRECTORY}/tmp.txt")
    endif()

    if(CTEST_PROJECT_NAME_SUPERBUILD)
      set(ctest_project_name_orig ${CTEST_PROJECT_NAME})
      set(CTEST_PROJECT_NAME ${CTEST_PROJECT_NAME_SUPERBUILD})
    endif()

    message("----------- [ Configure SuperBuild ] -----------")

    set_property(GLOBAL PROPERTY SubProject SuperBuild)
    set_property(GLOBAL PROPERTY Label SuperBuild)

    ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res OPTIONS "${SUPERBUILD_CONFIG_OPTIONS}")

    if(res)
      math(EXPR build_errors "${build_errors} + 1")
    endif()

    # Project.xml is generated during the superbuild configure step
    ctest_submit(FILES "${CTEST_BINARY_DIRECTORY}/Project.xml" RETRY_DELAY 3 RETRY_COUNT 3)

    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")

    ctest_submit(PARTS Configure RETRY_DELAY 3 RETRY_COUNT 3)

    # submit the update results *after* the submitting the Configure info,
    # otherwise CDash is somehow confused and cannot add the update info
    # to the superbuild project
    ctest_submit(PARTS Update RETRY_DELAY 3 RETRY_COUNT 3)

    # To get CTEST_PROJECT_SUBPROJECTS and CTEST_PROJECT_EXTERNALS definition
    include("${CTEST_BINARY_DIRECTORY}/CTestConfigSubProject.cmake")

    # Build top level (either all or the supplied targets at
    # superbuild level
    if(SUPERBUILD_TARGETS AND NOT build_errors)
      foreach(superbuild_target ${SUPERBUILD_TARGETS})

        message("----------- [ Build ${superbuild_target} - SuperBuild ] -----------")
        func_build_target(${superbuild_target} "${CTEST_BINARY_DIRECTORY}")

        # runs only tests that have a LABELS property matching "SuperBuild"
        func_test("SuperBuild" "${CTEST_BINARY_DIRECTORY}")
      endforeach()

      # HACK Unfortunately ctest_coverage ignores the build argument, back-up the original dirs
      file(READ "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" old_coverage_dirs)

      # explicitly build requested external projects as subprojects
      foreach(external_project_with_build_dir ${CTEST_PROJECT_EXTERNALS})

        string(REPLACE "^^" ";" external_project_with_build_dir_list "${external_project_with_build_dir}")
        list(GET external_project_with_build_dir_list 0 external_project_name)
        list(GET external_project_with_build_dir_list 1 external_project_builddir)

        set_property(GLOBAL PROPERTY SubProject ${external_project_name})
        set_property(GLOBAL PROPERTY Label ${external_project_name})

        message("----------- [ Build ${external_project_name} ] -----------")

        func_build_target("${external_project_name}" "${CTEST_BINARY_DIRECTORY}")

        if(NOT build_errors)
          # HACK Unfortunately ctest_coverage ignores the build argument, try to force it...
          file(READ "${CTEST_BINARY_DIRECTORY}/${external_project_builddir}/CMakeFiles/TargetDirectories.txt" mitk_build_coverage_dirs)
          file(APPEND "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" "${mitk_build_coverage_dirs}")

          message("----------- [ Test ${external_project_name} ] -----------")

          # runs only tests that have a LABELS property matching "${external_project_name}"
          func_test(${external_project_name} "${CTEST_BINARY_DIRECTORY}/${external_project_builddir}")

          # restore old coverage dirs
          file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" "${old_coverage_dirs}")
        endif()

      endforeach()

      # switch back to SuperBuild label
      set_property(GLOBAL PROPERTY SubProject SuperBuild)
      set_property(GLOBAL PROPERTY Label SuperBuild)

      message("----------- [ Finish SuperBuild ] -----------")
    else()
       message("----------- [ Build SuperBuild ] -----------")
    endif()

    if(NOT build_errors)
      # build everything at superbuild level which has not yet been built
      func_build_target("" "${CTEST_BINARY_DIRECTORY}")
    endif()

    # runs only tests that have a LABELS property matching "SuperBuild"
    #func_test("SuperBuild" "${CTEST_BINARY_DIRECTORY}")

    set(build_dir "${CTEST_BINARY_DIRECTORY}/${PROJECT_BUILD_DIR}")
    if(CTEST_PROJECT_NAME_SUPERBUILD)
      set(CTEST_PROJECT_NAME ${ctest_project_name_orig})
    endif()

    message("----------- [ Configure ${build_dir} ] -----------")
    # Configure target
    ctest_configure(BUILD "${build_dir}"
      OPTIONS "-DCTEST_USE_LAUNCHERS=${CTEST_USE_LAUNCHERS}"
      RETURN_VALUE res
    )
    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
    ctest_submit(PARTS Configure RETRY_DELAY 3 RETRY_COUNT 3)

    if(res)
      math(EXPR build_errors "${build_errors} + 1")
    endif()


    foreach(subproject ${CTEST_PROJECT_SUBPROJECTS})
      if(NOT build_errors)
        set_property(GLOBAL PROPERTY SubProject ${subproject})
        set_property(GLOBAL PROPERTY Label ${subproject})

        if(subproject MATCHES "Unlabeled")
          message("----------- [ Build All (Unlabeled) ] -----------")
          # Build target
          func_build_target("" "${build_dir}")
        else()
          message("----------- [ Build ${subproject} ] -----------")
          # Build target
          func_build_target(${subproject} "${build_dir}")
        endif()
      endif()
    endforeach()

    if(NOT build_errors)
      # HACK Unfortunately ctest_coverage ignores the build argument, try to force it...
      file(READ ${build_dir}/CMakeFiles/TargetDirectories.txt mitk_build_coverage_dirs)
      file(APPEND "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" "${mitk_build_coverage_dirs}")

      foreach(subproject ${CTEST_PROJECT_SUBPROJECTS})
        set_property(GLOBAL PROPERTY SubProject ${subproject})
        set_property(GLOBAL PROPERTY Label ${subproject})
        message("----------- [ Test ${subproject} ] -----------")

        # runs only tests that have a LABELS property matching "${subproject}"
        func_test(${subproject} "${build_dir}")
      endforeach()
    endif()

    # Build any additional target which is not build by "all"
    # i.e. the "package" target
    if(CTEST_PROJECT_ADDITIONAL_TARGETS AND NOT build_errors)
      foreach(additional_target ${CTEST_PROJECT_ADDITIONAL_TARGETS})
        set_property(GLOBAL PROPERTY SubProject ${additional_target})
        set_property(GLOBAL PROPERTY Label ${additional_target})

        message("----------- [ Build ${additional_target} ] -----------")
        func_build_target(${additional_target} "${build_dir}")

        message("----------- [ Test ${additional_target} ] -----------")
        # runs only tests that have a LABELS property matching "${subproject}"
        func_test(${additional_target} "${build_dir}")
      endforeach()
    endif()

    if(WITH_DOCUMENTATION)
      message("----------- [ Build Documentation ] -----------")
      set(ctest_use_launchers_orig ${CTEST_USE_LAUNCHERS})
      set(CTEST_USE_LAUNCHERS 0)
      # Build Documentation target
      set_property(GLOBAL PROPERTY SubProject Documentation)
      set_property(GLOBAL PROPERTY Label Documentation)
      func_build_target("doc" "${build_dir}")
      set(CTEST_USE_LAUNCHERS ${ctest_use_launchers_orig})
    endif()

    set_property(GLOBAL PROPERTY SubProject SuperBuild)
    set_property(GLOBAL PROPERTY Label SuperBuild)

    # Global coverage ...
    if(WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
      message("----------- [ Global coverage ] -----------")
      ctest_coverage(BUILD "${build_dir}" APPEND)
      ctest_submit(PARTS Coverage RETRY_DELAY 3 RETRY_COUNT 3)
    endif()

    # Global dynamic analysis ...
    if(WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
      message("----------- [ Global memcheck ] -----------")
      ctest_memcheck(BUILD "${build_dir}")
      ctest_submit(PARTS MemCheck RETRY_DELAY 3 RETRY_COUNT 3)
    endif()

    # Note should be at the end
    ctest_submit(PARTS Notes RETRY_DELAY 3 RETRY_COUNT 3)

    # Send status to the "CDash Web Admin"
    if(CDASH_ADMIN_URL_PREFIX)
      set(cdash_admin_url "${CDASH_ADMIN_URL_PREFIX}/cdashadmin-web/index.php?pw=4da12ca9c06d46d3171d7f73974c900f")
      string(REGEX REPLACE ".*\\?project=(.*)&?" "\\1" _ctest_project "${CTEST_DROP_LOCATION}")
      file(DOWNLOAD
           "${cdash_admin_url}&action=submit&name=${_build_name_escaped}&hasTestErrors=${test_errors}&hasBuildErrors=${build_errors}&hasBuildWarnings=${build_warnings}&ctestDropSite=${CTEST_DROP_SITE}&ctestProject=${_ctest_project}"
           "${CTEST_BINARY_DIRECTORY}/cdashadmin.txt"
           STATUS status
           )
      list(GET status 0 error_code)
      list(GET status 1 error_msg)
      if(error_code)
        message(FATAL_ERROR "error: Failed to communicate with cdashadmin-web - ${error_msg}")
      endif()
    endif()

  endif()

  # Clear the CTEST_CHECKOUT_COMMAND variable to prevent continuous clients
  # to try to checkout again
  set(CTEST_CHECKOUT_COMMAND "")

endmacro()

if(SCRIPT_MODE STREQUAL "continuous")
  while(1)
    run_ctest()
    # Loop no faster than once every 5 minutes
    message("Wait for 5 minutes ...")
    ctest_sleep(300)
  endwhile()
else()
  run_ctest()
endif()

