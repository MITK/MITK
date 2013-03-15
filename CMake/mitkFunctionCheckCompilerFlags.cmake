#
# Helper macro allowing to check if the given flags are supported
# by the underlying build tool
#
# If the flag(s) is/are supported, they will be appended to the string identified by RESULT_VAR
#
# Usage:
#   mitkFunctionCheckCompilerFlags(FLAGS_TO_CHECK VALID_FLAGS_VAR)
#
# The above example uses the C++ compiler to check the flags. To individually check with
# the C and C++ compiler, use:
#
#   mitkFunctionCheckCompilerFlags2(FLAGS_TO_CHECK VALID_C_FLAGS_VAR VALID_CXX_FLAGS_VAR)
#
# Example:
#
#   set(myflags)
#   mitkFunctionCheckCompilerFlags("-fprofile-arcs" myflags)
#   message(1-myflags:${myflags})
#   mitkFunctionCheckCompilerFlags("-fauto-bugfix" myflags)
#   message(2-myflags:${myflags})
#   mitkFunctionCheckCompilerFlags("-Wall" myflags)
#   message(1-myflags:${myflags})
#
#   The output will be:
#    1-myflags: -fprofile-arcs
#    2-myflags: -fprofile-arcs
#    3-myflags: -fprofile-arcs -Wall

include(TestCXXAcceptsFlag)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

function(mitkFunctionCheckCompilerFlags CXX_FLAG_TO_TEST RESULT_VAR)

  if(CXX_FLAG_TO_TEST STREQUAL "")
    message(FATAL_ERROR "CXX_FLAG_TO_TEST shouldn't be empty")
  endif()

  # Internally, the macro CMAKE_CXX_ACCEPTS_FLAG calls TRY_COMPILE. To avoid
  # the cost of compiling the test each time the project is configured, the variable set by
  # the macro is added to the cache so that following invocation of the macro with
  # the same variable name skip the compilation step.
  # For that same reason, ctkFunctionCheckCompilerFlags function appends a unique suffix to
  # the HAS_FLAG variable. This suffix is created using a 'clean version' of the flag to test.
  string(REGEX REPLACE "[,= \\$\\+\\*\\{\\}\\(\\)\\#-]" "" suffix ${CXX_FLAG_TO_TEST})
  CHECK_CXX_ACCEPTS_FLAG(${CXX_FLAG_TO_TEST} HAS_FLAG_${suffix})

  if(HAS_FLAG_${suffix})
    set(${RESULT_VAR} "${${RESULT_VAR}} ${CXX_FLAG_TO_TEST}" PARENT_SCOPE)
  endif()

endfunction()

function(mitkFunctionCheckCompilerFlags2 FLAG_TO_TEST C_RESULT_VAR CXX_RESULT_VAR)

  if(FLAG_TO_TEST STREQUAL "")
    message(FATAL_ERROR "FLAG_TO_TEST shouldn't be empty")
  endif()

  # Internally, the macro CMAKE_CXX_ACCEPTS_FLAG calls TRY_COMPILE. To avoid
  # the cost of compiling the test each time the project is configured, the variable set by
  # the macro is added to the cache so that following invocation of the macro with
  # the same variable name skip the compilation step.
  # For that same reason, ctkFunctionCheckCompilerFlags function appends a unique suffix to
  # the HAS_FLAG variable. This suffix is created using a 'clean version' of the flag to test.
  string(REGEX REPLACE "[,= \\$\\+\\*\\{\\}\\(\\)\\#-]" "" suffix ${FLAG_TO_TEST})
  CHECK_CXX_COMPILER_FLAG(${FLAG_TO_TEST} HAS_CXX_FLAG_${suffix})

  if(HAS_CXX_FLAG_${suffix})
    set(${CXX_RESULT_VAR} "${${CXX_RESULT_VAR}} ${FLAG_TO_TEST}" PARENT_SCOPE)
  endif()

  CHECK_C_COMPILER_FLAG(${FLAG_TO_TEST} HAS_C_FLAG_${suffix})

  if(HAS_C_FLAG_${suffix})
    set(${C_RESULT_VAR} "${${C_RESULT_VAR}} ${FLAG_TO_TEST}" PARENT_SCOPE)
  endif()

endfunction()
