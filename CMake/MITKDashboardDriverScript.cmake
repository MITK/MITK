
#
# Included from a dashboard script, this cmake file will drive the configure and build
# steps of MITK
#

#-----------------------------------------------------------------------------
# The following variable are expected to be define in the top-level script:
set(expected_variables
  ADDITIONNAL_CMAKECACHE_OPTION
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
  CTEST_PROJECT_NAME
  CTEST_SOURCE_DIRECTORY
  CTEST_BINARY_DIRECTORY
  CTEST_BUILD_NAME
  SCRIPT_MODE
  CTEST_COVERAGE_COMMAND
  CTEST_MEMORYCHECK_COMMAND
  CTEST_GIT_COMMAND
  QT_QMAKE_EXECUTABLE
  )
if(WITH_DOCUMENTATION)
  list(APPEND expected_variables DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)
endif()

foreach(var ${expected_variables})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} should be defined in top-level script !")
  endif()
endforeach()

# If the dashscript doesn't define a GIT_REPOSITORY variable, let's define it here.
if (NOT DEFINED GIT_REPOSITORY OR GIT_REPOSITORY STREQUAL "")
  set(GIT_REPOSITORY git@mbits:MITK)
endif()

# Should binary directory be cleaned?
set(empty_binary_directory FALSE)

# Attempt to build and test also if 'ctest_update' returned an error
set(force_build FALSE)

# Set model options
set(model "")
if (SCRIPT_MODE STREQUAL "experimental")
  set(empty_binary_directory FALSE)
  set(force_build TRUE)
  set(model Experimental)
elseif (SCRIPT_MODE STREQUAL "continuous")
  set(empty_binary_directory TRUE)
  set(force_build FALSE)
  set(model Continuous)
elseif (SCRIPT_MODE STREQUAL "nightly")
  set(empty_binary_directory TRUE)
  set(force_build TRUE)
  set(model Nightly)
else()
  message(FATAL_ERROR "Unknown script mode: '${SCRIPT_MODE}'. Script mode should be either 'experimental', 'continuous' or 'nightly'")
endif()

#message("script_mode:${SCRIPT_MODE}")
#message("model:${model}")
#message("empty_binary_directory:${empty_binary_directory}")
#message("force_build:${force_build}")

set(CTEST_USE_LAUNCHERS 1)

if(empty_binary_directory)
  message("Directory ${CTEST_BINARY_DIRECTORY} cleaned !")
  ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
endif()

if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}")
  set(CTEST_CHECKOUT_COMMAND "${CTEST_GIT_COMMAND} clone ${GIT_REPOSITORY} ${CTEST_SOURCE_DIRECTORY}")
endif()

set(CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}")

#
# run_ctest macro
#
MACRO(run_ctest)
  ctest_start(${model})
  ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" RETURN_VALUE res)

  # force a build if this is the first run and the build dir is empty
  if(NOT EXISTS "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt")
    message("First time build - Initialize CMakeCache.txt")
    set(res 1)

    # Write initial cache.
    file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CTEST_USE_LAUNCHERS:BOOL=${CTEST_USE_LAUNCHERS}
BUILD_TESTING:BOOL=TRUE
BLUEBERRY_BUILD_ALL_PLUGINS:BOOL=TRUE
MITK_BUILD_ALL_PLUGINS:BOOL=TRUE
CMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}
QT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
SUPERBUILD_EXCLUDE_MITKBUILD_TARGET:BOOL=TRUE
WITH_COVERAGE:BOOL=${WITH_COVERAGE}
#DOCUMENTATION_TARGET_IN_ALL:BOOL=${WITH_DOCUMENTATION}
DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY:PATH=${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}
${ADDITIONNAL_CMAKECACHE_OPTION}
")
  endif()
  
  if (res GREATER 0 OR force_build)
  
    ctest_submit(PARTS Update)
      
    message("Configure SuperBuild")
    
    set_property(GLOBAL PROPERTY SubProject SuperBuild)
    set_property(GLOBAL PROPERTY Label SuperBuild)
     
    ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}")
    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
    ctest_submit(PARTS Configure)
    ctest_submit(FILES "${CTEST_BINARY_DIRECTORY}/Project.xml")

    # Build top level
    message("----------- [ Build SuperBuild ] -----------")
    ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" APPEND)
    ctest_submit(PARTS Build)
    
    ctest_test(
      BUILD "${CTEST_BINARY_DIRECTORY}" 
      INCLUDE_LABEL "SuperBuild"
      PARALLEL_LEVEL 8
      EXCLUDE ${TEST_TO_EXCLUDE_REGEX})
    # runs only tests that have a LABELS property matching "${subproject}"
    ctest_submit(PARTS Test)
      
    set(mitk_build_dir "${CTEST_BINARY_DIRECTORY}/MITK-build")
    
    set(CTEST_PROJECT_SUBPROJECTS MITK)
    
    foreach(subproject ${CTEST_PROJECT_SUBPROJECTS})
      set_property(GLOBAL PROPERTY SubProject ${subproject})
      set_property(GLOBAL PROPERTY Label ${subproject})
      message("----------- [ Build ${subproject} ] -----------")
    
      # Configure target
      ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}")
      ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
      ctest_submit(PARTS Configure)
     
      # Build target
      set(CTEST_BUILD_TARGET "${subproject}")
      ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" APPEND)
      ctest_submit(PARTS Build)
      
      message("----------- [ Test ${subproject} ] -----------")

      ctest_test(
        BUILD "${mitk_build_dir}"
        APPEND
        # INCLUDE_LABEL "${subproject}"
        PARALLEL_LEVEL 8
        EXCLUDE ${TEST_TO_EXCLUDE_REGEX})
      # runs only tests that have a LABELS property matching "${subproject}"
      
      ctest_submit(PARTS Test)

      # Coverage per sub-project
      if (WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
        message("----------- [ Coverage ${subproject} ] -----------")
        ctest_coverage(BUILD "${mitk_build_dir}") # LABELS "${subproject}")
        ctest_submit(PARTS Coverage)
      endif ()

      #if (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
      #  ctest_memcheck(BUILD "${mitk_build_dir}" INCLUDE_LABEL "${subproject}")
      #  ctest_submit(PARTS MemCheck)
      #endif ()
    endforeach()
    
    if (WITH_DOCUMENTATION)
      message("----------- [ Build Documentation ] -----------")
      set(CTEST_USE_LAUNCHERS 0)
      # Build Documentation target
      set_property(GLOBAL PROPERTY SubProject Documentation)
      set_property(GLOBAL PROPERTY Label Documentation)
      set(CTEST_BUILD_TARGET "doc")
      ctest_build(BUILD "${mitk_build_dir}" APPEND)
      ctest_submit(PARTS Build)
      set(CTEST_USE_LAUNCHERS 1)
    endif()
    
    # HACK Unfortunately ctest_coverage ignores the build argument, try to force it...
    file(READ ${mitk_build_dir}/CMakeFiles/TargetDirectories.txt mitk_build_coverage_dirs)
    file(APPEND "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" "${mitk_build_coverage_dirs}")
    
    set_property(GLOBAL PROPERTY SubProject SuperBuild)
    set_property(GLOBAL PROPERTY Label SuperBuild)
    
    # Global coverage ... 
    if (WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
      message("----------- [ Global coverage ] -----------")
      ctest_coverage(BUILD "${mitk_build_dir}" APPEND)
      ctest_submit(PARTS Coverage)
    endif ()
    
    # Global dynamic analysis ...
    if (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
        message("----------- [ Global memcheck ] -----------")
        ctest_memcheck(BUILD "${mitk_build_dir}")
        ctest_submit(PARTS MemCheck)
      endif ()
    
    # Note should be at the end
    ctest_submit(PARTS Notes)
  
  endif()
endmacro()

if(SCRIPT_MODE STREQUAL "continuous")
  while(${CTEST_ELAPSED_TIME} LESS 68400)
    set(START_TIME ${CTEST_ELAPSED_TIME})
    run_ctest()
    # Loop no faster than once every 5 minutes
    message("Wait for 5 minutes ...")
    ctest_sleep(${START_TIME} 300 ${CTEST_ELAPSED_TIME})
  endwhile()
else()
  run_ctest()
endif()

