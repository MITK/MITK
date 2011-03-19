
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
  CTEST_SOURCE_DIRECTORY
  CTEST_BINARY_DIRECTORY
  CTEST_BUILD_NAME
  SCRIPT_MODE
  CTEST_COVERAGE_COMMAND
  CTEST_MEMORYCHECK_COMMAND
  CTEST_GIT_COMMAND
  QT_QMAKE_EXECUTABLE
  PROJECT_BUILD_DIR
  SUPERBUILD_TARGETS
  )

foreach(var ${expected_variables})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} should be defined in top-level script !")
  endif()
endforeach()

if (NOT DEFINED GIT_BRANCH OR GIT_BRANCH STREQUAL "")
  set(GIT_BRANCH "")
else()
  set(GIT_BRANCH "-b ${GIT_BRANCH}")
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
  set(empty_binary_directory FALSE)
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

if(NOT DARTCLIENT_ERROR_FILE)
  set(DARTCLIENT_ERROR_FILE "${CTEST_BINARY_DIRECTORY}/dartclient_error.txt")
endif()

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

#
# run_ctest macro
#
MACRO(run_ctest)
  set(dartclient_error)

  set_property(GLOBAL PROPERTY SubProject SuperBuild)
  set_property(GLOBAL PROPERTY Label SuperBuild)

  ctest_start(${model})
  ctest_update(SOURCE "${CTEST_CHECKOUT_DIR}" RETURN_VALUE res)
  
  if(COMMAND MITK_OVERRIDE_FORCE_BUILD)
    MITK_OVERRIDE_FORCE_BUILD(force_build)
  endif()

  # force a build if this is the first run and the build dir is empty
  if(NOT EXISTS "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt")
    message("First time build - Initialize CMakeCache.txt")
    set(res 1)

    # Write initial cache.
    file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CTEST_USE_LAUNCHERS:BOOL=${CTEST_USE_LAUNCHERS}
BUILD_TESTING:BOOL=TRUE
BLUEBERRY_BUILD_TESTING:BOOL=TRUE
BLUEBERRY_BUILD_ALL_PLUGINS:BOOL=TRUE
MITK_BUILD_ALL_PLUGINS:BOOL=TRUE
MITK_CTEST_SCRIPT_MODE:BOOL=TRUE
CMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}
QT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
SUPERBUILD_EXCLUDE_MITKBUILD_TARGET:BOOL=TRUE
WITH_COVERAGE:BOOL=${WITH_COVERAGE}
MITK_DOXYGEN_OUTPUT_DIR:PATH=${MITK_DOXYGEN_OUTPUT_DIR}
${ADDITIONNAL_CMAKECACHE_OPTION}
")
  endif()
  
  if(NOT dartclient_error AND res LESS 0)
    # update error
    set(dartclient_error "Update or checkout error") 
  endif()
  
  if(res GREATER 0 OR force_build)
  
    ctest_submit(PARTS Update)
      
    message("----------- [ Configure SuperBuild ] -----------")
    
    ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}"
      OPTIONS "-DCTEST_USE_LAUNCHERS=${CTEST_USE_LAUNCHERS}"
      RETURN_VALUE res
    )
    
    if(NOT dartclient_error AND res)
      set(dartclient_error "SuperBuild configure error (code ${res})")
    endif()
    
    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
    ctest_submit(PARTS Configure)
    ctest_submit(FILES "${CTEST_BINARY_DIRECTORY}/Project.xml")
    
    # To get CTEST_PROJECT_SUBPROJECTS and CTEST_PROJECT_EXTERNALS definition
    include("${CTEST_BINARY_DIRECTORY}/CTestConfigSubProject.cmake")

    # Build top level (either all or the supplied targets at
    # superbuild level
    if(SUPERBUILD_TARGETS)
      foreach(superbuild_target ${SUPERBUILD_TARGETS})
        message("----------- [ Build ${superbuild_target} - SuperBuild ] -----------")
        set(CTEST_BUILD_TARGET "${superbuild_target}")
        ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" APPEND
          RETURN_VALUE res
          NUMBER_ERRORS num_errors
          NUMBER_WARNINGS num_warnings)
        ctest_submit(PARTS Build)
        
        if(NOT dartclient_error AND res)
          set(dartclient_error "Build error for ${superbuild_target} (code ${res}): Warnings ${num_warnings}, Errors ${num_errors}")
        elseif(NOT dartclient_error AND num_warnings)
          set(dartclient_error "Warnings for ${superbuild_target}: ${num_warnings}")
        endif()
        
        # runs only tests that have a LABELS property matching "SuperBuild"
        ctest_test(
          BUILD "${CTEST_BINARY_DIRECTORY}"
          INCLUDE_LABEL "SuperBuild"
          PARALLEL_LEVEL 8
          EXCLUDE ${TEST_TO_EXCLUDE_REGEX}
          RETURN_VALUE res
          )
        
        ctest_submit(PARTS Test)
        
        if(NOT dartclient_error AND res)
          set(dartclient_error "Test failures for ${superbuild_target} (code ${res})")
        endif()
      endforeach()
      
      # HACK Unfortunately ctest_coverage ignores the build argument, back-up the original dirs
      file(READ "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" old_coverage_dirs)
      
      # explicitly build requested external projects as subprojects
      foreach(external_project_with_build_dir ${CTEST_PROJECT_EXTERNALS})
        string(REPLACE "^^" ";" external_project_with_build_dir_list "${external_project_with_build_dir}")
        list(GET external_project_with_build_dir_list 0 external_project)
        list(GET external_project_with_build_dir_list 1 external_project_build_dir)
                
        set_property(GLOBAL PROPERTY SubProject ${external_project})
        set_property(GLOBAL PROPERTY Label ${external_project})
        message("----------- [ Build ${external_project} ] -----------")
       
        # Build target
        set(CTEST_BUILD_TARGET "${external_project}")
        ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" APPEND
          RETURN_VALUE res
          NUMBER_ERRORS num_errors
          NUMBER_WARNINGS num_warnings)
        ctest_submit(PARTS Build)
        
        if(NOT dartclient_error AND res)
          set(dartclient_error "Build error for ${external_project} (code ${res}): Warnings ${num_warnings}, Errors ${num_errors}")
        elseif(NOT dartclient_error AND num_warnings)
          set(dartclient_error "Warnings for ${external_project}: ${num_warnings}")
        endif()
        
        # HACK Unfortunately ctest_coverage ignores the build argument, try to force it...
        file(READ "${CTEST_BINARY_DIRECTORY}/${external_project_build_dir}/CMakeFiles/TargetDirectories.txt" mitk_build_coverage_dirs)
        file(APPEND "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" "${mitk_build_coverage_dirs}")
        
        message("----------- [ Test ${external_project} ] -----------")

        # runs only tests that have a LABELS property matching "${external_project}"
        ctest_test(
          BUILD "${CTEST_BINARY_DIRECTORY}/${external_project_build_dir}"
          APPEND
          INCLUDE_LABEL "${external_project}"
          PARALLEL_LEVEL 8
          EXCLUDE ${TEST_TO_EXCLUDE_REGEX}
          RETURN_VALUE res
          )
        
        ctest_submit(PARTS Test)
        
        if(NOT dartclient_error AND res)
          set(dartclient_error "Test failures for ${external_project} (code ${res})")
        endif()

        # Coverage per external project
        if (WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
          message("----------- [ Coverage ${external_project} ] -----------")
          ctest_coverage(BUILD "${CTEST_BINARY_DIRECTORY}/${external_project_build_dir}" LABELS "${external_project}")
          ctest_submit(PARTS Coverage)
        endif ()

        #if (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
        #  ctest_memcheck(BUILD "${build_dir}" INCLUDE_LABEL "${subproject}")
        #  ctest_submit(PARTS MemCheck)
        #endif ()
        
        # restore old coverage dirs
        file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" "${old_coverage_dirs}")
      
      endforeach()

      # switch back to SuperBuild label
      set_property(GLOBAL PROPERTY SubProject SuperBuild)
      set_property(GLOBAL PROPERTY Label SuperBuild)
      
      message("----------- [ Finish SuperBuild ] -----------")
    else()
       message("----------- [ Build SuperBuild ] -----------")
    endif()
    
    # build everything at superbuild level which has not yet been built
    set(CTEST_BUILD_TARGET)
    ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" APPEND
      RETURN_VALUE res
      NUMBER_ERRORS num_errors
      NUMBER_WARNINGS num_warnings)
      
    ctest_submit(PARTS Build)
    
    if(NOT dartclient_error AND res)
      set(dartclient_error "Build error for SuperBuild (code ${res}): Warnings ${num_warnings}, Errors ${num_errors}")
    elseif(NOT dartclient_error AND num_warnings)
      set(dartclient_error "Warnings for SuperBuild: ${num_warnings}")
    endif()
    
    # runs only tests that have a LABELS property matching "SuperBuild" 
    ctest_test(
      BUILD "${CTEST_BINARY_DIRECTORY}" 
      INCLUDE_LABEL "SuperBuild"
      PARALLEL_LEVEL 8
      EXCLUDE ${TEST_TO_EXCLUDE_REGEX}
      RETURN_VALUE res
      )
    
    ctest_submit(PARTS Test)
    
    if(NOT dartclient_error AND res)
      set(dartclient_error "Test failures for SuperBuild (code ${res})")
    endif()
    
    set(build_dir "${CTEST_BINARY_DIRECTORY}/${PROJECT_BUILD_DIR}")
    
    message("----------- [ Configure ${build_dir} ] -----------")
    # Configure target
    ctest_configure(BUILD "${build_dir}"
      OPTIONS "-DCTEST_USE_LAUNCHERS=${CTEST_USE_LAUNCHERS}"
      RETURN_VALUE res
    )
    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
    ctest_submit(PARTS Configure)
    
    if(NOT dartclient_error AND res)
      set(dartclient_error "Configure error in ${build_dir} (code ${res})")
    endif()
    
    foreach(subproject ${CTEST_PROJECT_SUBPROJECTS})
      set_property(GLOBAL PROPERTY SubProject ${subproject})
      set_property(GLOBAL PROPERTY Label ${subproject})
      message("----------- [ Build ${subproject} ] -----------")
     
      # Build target
      set(CTEST_BUILD_TARGET "${subproject}")
      ctest_build(BUILD "${build_dir}" APPEND
        RETURN_VALUE res
        NUMBER_ERRORS num_errors
        NUMBER_WARNINGS num_warnings)
      ctest_submit(PARTS Build)
      
      if(NOT dartclient_error AND res)
        set(dartclient_error "Build error for ${subproject} (code ${res}): Warnings ${num_warnings}, Errors ${num_errors}")
      elseif(NOT dartclient_error AND num_warnings)
        set(dartclient_error "Warnings for ${subproject}: ${num_warnings}")
      endif()
    endforeach()
    
    # Build the rest of the project
    set_property(GLOBAL PROPERTY SubProject SuperBuild)
    set_property(GLOBAL PROPERTY Label SuperBuild)
    
    message("----------- [ Build All ] -----------")
    set(CTEST_BUILD_TARGET)
    ctest_build(BUILD "${build_dir}" APPEND
      RETURN_VALUE res
      NUMBER_ERRORS num_errors
      NUMBER_WARNINGS num_warnings)
    ctest_submit(PARTS Build)
  
    if(NOT dartclient_error AND res)
      set(dartclient_error "Build error (code ${res}): Warnings ${num_warnings}, Errors ${num_errors}")
    elseif(NOT dartclient_error AND num_warnings)
      set(dartclient_error "Warnings: ${num_warnings}")
    endif()
    
    # HACK Unfortunately ctest_coverage ignores the build argument, try to force it...
    file(READ ${build_dir}/CMakeFiles/TargetDirectories.txt mitk_build_coverage_dirs)
    file(APPEND "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" "${mitk_build_coverage_dirs}")
    
    foreach(subproject ${CTEST_PROJECT_SUBPROJECTS})
      set_property(GLOBAL PROPERTY SubProject ${subproject})
      set_property(GLOBAL PROPERTY Label ${subproject})
      message("----------- [ Test ${subproject} ] -----------")

      # runs only tests that have a LABELS property matching "${subproject}"
      ctest_test(
        BUILD "${build_dir}"
        APPEND
        INCLUDE_LABEL "${subproject}"
        PARALLEL_LEVEL 8
        EXCLUDE ${TEST_TO_EXCLUDE_REGEX}
        RETURN_VALUE res
        )
      
      ctest_submit(PARTS Test)
      
      if(NOT dartclient_error AND res)
        set(dartclient_error "Test failures for ${subproject} (code ${res})")
      endif()

      # Coverage per sub-project
      if (WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
        message("----------- [ Coverage ${subproject} ] -----------")
        ctest_coverage(BUILD "${build_dir}" LABELS "${subproject}")
        ctest_submit(PARTS Coverage)
      endif ()

      #if (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
      #  ctest_memcheck(BUILD "${build_dir}" INCLUDE_LABEL "${subproject}")
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
      ctest_build(BUILD "${build_dir}" APPEND)
      ctest_submit(PARTS Build)
      set(CTEST_USE_LAUNCHERS 1)
    endif()
    
    set_property(GLOBAL PROPERTY SubProject SuperBuild)
    set_property(GLOBAL PROPERTY Label SuperBuild)
    
    # Global coverage ... 
    if (WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
      message("----------- [ Global coverage ] -----------")
      ctest_coverage(BUILD "${build_dir}" APPEND)
      ctest_submit(PARTS Coverage)
    endif ()
    
    # Global dynamic analysis ...
    if (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
      message("----------- [ Global memcheck ] -----------")
      ctest_memcheck(BUILD "${build_dir}")
      ctest_submit(PARTS MemCheck)
    endif ()
    
    # Note should be at the end
    ctest_submit(PARTS Notes)
  
  endif()
  
  # Clear the CTEST_CHECKOUT_COMMAND variable to prevent continuous clients
  # to try to checkout again
  set(CTEST_CHECKOUT_COMMAND "")
  
  if(dartclient_error)
    file(WRITE ${DARTCLIENT_ERROR_FILE} "${dartclient_error}")
  else()
    file(REMOVE ${DARTCLIENT_ERROR_FILE})
  endif()
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

