# AssertJsonContains.cmake
#
# Runs a command and then requires the produced JSON file to contain every
# given substring. Used by the DICOMVolumeDiagnostics CLI tests, where the
# assertion is about the report's content rather than only its exit code.
#
# Expected variables:
#   CMD       the executable to run
#   ARGS      its arguments, as a CMake list
#   JSON      the file the command is expected to write
#   CONTAINS  substrings that must all appear in JSON, as a CMake list

if(EXISTS "${JSON}")
  file(REMOVE "${JSON}")
endif()

execute_process(COMMAND ${CMD} ${ARGS} RESULT_VARIABLE _code OUTPUT_QUIET ERROR_VARIABLE _err)

if(NOT _code EQUAL 0)
  message(FATAL_ERROR "Expected exit code 0 but got ${_code}.\n${_err}")
endif()

if(NOT EXISTS "${JSON}")
  message(FATAL_ERROR "The command exited 0 but wrote no report to ${JSON}.")
endif()

file(READ "${JSON}" _report)

foreach(_needle IN LISTS CONTAINS)
  string(FIND "${_report}" "${_needle}" _at)
  if(_at EQUAL -1)
    message(FATAL_ERROR "The report does not contain '${_needle}'.\nReport:\n${_report}")
  endif()
endforeach()
