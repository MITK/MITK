# AssertExitCode.cmake
#
# cmake -P wrapper for ctest add_test() that asserts a *specific* exit
# code from the wrapped command. CTest's WILL_FAIL only checks "non-zero"
# and PASS_REGULAR_EXPRESSION matches output text, neither of which can
# distinguish exit 4 from exit 5 from exit 8 -- which is exactly what the
# PETSUVCalculation CLI smoke tests need to pin.
#
# Invocation:
#   cmake -DEXPECTED=<n>
#         -DCMD=<absolute path to executable>
#         -DARGS="<arg1>;<arg2>;..."        # CMake list (semicolons)
#         [-DWORKING_DIRECTORY=<dir>]
#         [-DEXPECT_OUTPUT=<regex>]         # must match stdout or stderr
#         [-DREJECT_OUTPUT=<regex>]         # must match neither
#         -P AssertExitCode.cmake
#
# Exit 0 iff the wrapped command's exit code equals EXPECTED and the
# optional output assertions hold. Otherwise exits with a small non-zero
# code so ctest reports failure.
#
# The output assertions exist for contracts that are not expressible as an
# exit code: a run can succeed and still owe the operator a diagnostic.

if(NOT DEFINED EXPECTED)
  message(FATAL_ERROR "AssertExitCode.cmake: EXPECTED not set.")
endif()
if(NOT DEFINED CMD)
  message(FATAL_ERROR "AssertExitCode.cmake: CMD not set.")
endif()
if(NOT DEFINED ARGS)
  set(ARGS "")
endif()

if(DEFINED WORKING_DIRECTORY)
  execute_process(
    COMMAND "${CMD}" ${ARGS}
    WORKING_DIRECTORY "${WORKING_DIRECTORY}"
    RESULT_VARIABLE actual_exit
    OUTPUT_VARIABLE captured_stdout
    ERROR_VARIABLE  captured_stderr)
else()
  execute_process(
    COMMAND "${CMD}" ${ARGS}
    RESULT_VARIABLE actual_exit
    OUTPUT_VARIABLE captured_stdout
    ERROR_VARIABLE  captured_stderr)
endif()

set(_captured "${captured_stdout}${captured_stderr}")

function(_assert_exit_code_dump)
  message("---- captured stdout ----")
  message("${captured_stdout}")
  message("---- captured stderr ----")
  message("${captured_stderr}")
endfunction()

if(NOT actual_exit STREQUAL EXPECTED)
  _assert_exit_code_dump()
  message(FATAL_ERROR
    "AssertExitCode: '${CMD}' exited ${actual_exit}, expected ${EXPECTED}.")
endif()

if(DEFINED EXPECT_OUTPUT AND NOT _captured MATCHES "${EXPECT_OUTPUT}")
  _assert_exit_code_dump()
  message(FATAL_ERROR
    "AssertExitCode: '${CMD}' produced no output matching '${EXPECT_OUTPUT}'.")
endif()

if(DEFINED REJECT_OUTPUT AND _captured MATCHES "${REJECT_OUTPUT}")
  _assert_exit_code_dump()
  message(FATAL_ERROR
    "AssertExitCode: '${CMD}' produced output matching '${REJECT_OUTPUT}', "
    "which this case forbids.")
endif()
