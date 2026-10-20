# apply_patches.cmake - run MITK's fixes for provisioned Boost sources.
#
# Every script in PATCH_DIR is run with BOOST_SRC, in name order. A script edits
# the sources of the library it fixes, must do nothing if that library was not
# provisioned or is fixed already (the step reruns), and should fail loudly if
# its anchor is gone, so a Boost upgrade reveals fixes that became obsolete.
#
# Invoke (cmake -P):
#   cmake -DBOOST_SRC=<superproject> -DPATCH_DIR=<patches> -P apply_patches.cmake

cmake_minimum_required(VERSION 3.20)

foreach(_var BOOST_SRC PATCH_DIR)
  if(NOT DEFINED ${_var})
    message(FATAL_ERROR "apply_patches.cmake: ${_var} is required")
  endif()
endforeach()

file(GLOB _patches "${PATCH_DIR}/*.cmake")
list(SORT _patches)

foreach(_patch IN LISTS _patches)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -DBOOST_SRC=${BOOST_SRC} -P "${_patch}"
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "Boost: patch ${_patch} failed (exit ${_rc})")
  endif()
endforeach()
