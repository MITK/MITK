# fetch_one.cmake - sparse + shallow, pinned fetch of a single Boost module.
#
# Materializes only include/src/cmake/extra of one library at its release-pinned
# commit, into <BOOST_SRC>/libs/<MODULE>. Pure git, cross-platform; this is the
# single source of the fetch logic (both the sequential and the parallel paths
# invoke it). Skips work if the module is already checked out at the pinned SHA.
#
# Invoke:
#   cmake -DBOOST_SRC=<superproject> -DMODULE=<name> \
#         -DBOOST_URL=<repo-url> -DBOOST_SHA=<commit> -P fetch_one.cmake

cmake_minimum_required(VERSION 3.20)

foreach(_var BOOST_SRC MODULE BOOST_URL BOOST_SHA)
  if(NOT DEFINED ${_var})
    message(FATAL_ERROR "fetch_one.cmake: ${_var} is required")
  endif()
endforeach()

find_program(GIT_EXECUTABLE NAMES git REQUIRED)
set(dir "${BOOST_SRC}/libs/${MODULE}")

# Incremental: already at the pinned commit -> nothing to fetch.
if(EXISTS "${dir}/.git")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${dir}" rev-parse HEAD
    OUTPUT_VARIABLE current OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET RESULT_VARIABLE rc)
  if(rc EQUAL 0 AND current STREQUAL "${BOOST_SHA}")
    return()
  endif()
endif()

function(_git)
  execute_process(COMMAND "${GIT_EXECUTABLE}" ${ARGN} RESULT_VARIABLE rc)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "git ${ARGN} failed (exit ${rc})")
  endif()
endfunction()

file(REMOVE_RECURSE "${dir}")
_git(init -q "${dir}")
_git(-C "${dir}" remote add origin "${BOOST_URL}")
_git(-C "${dir}" sparse-checkout init --cone)
_git(-C "${dir}" sparse-checkout set include src cmake extra)
_git(-C "${dir}" fetch -q --depth 1 --filter=blob:none origin "${BOOST_SHA}")
_git(-C "${dir}" checkout -q --detach FETCH_HEAD)
