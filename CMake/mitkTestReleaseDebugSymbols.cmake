# Verifies that a binary built with MITK_RELEASE_DEBUG_SYMBOLS carries DWARF
# line tables. Run through ctest as mitkReleaseDebugSymbolsTest.
#
# A debug flag the compiler accepts but that generates nothing, such as
# -gsplit-dwarf without -g on GCC, builds and links fine and only shows when
# a crash dump cannot be decoded, long after the release shipped.
#
# Required variables (passed with -D):
#   MITK_DEBUG_SYMBOLS_BINARY   ELF file to inspect
#   MITK_DEBUG_SYMBOLS_OBJDUMP  objdump executable

foreach(_required MITK_DEBUG_SYMBOLS_BINARY MITK_DEBUG_SYMBOLS_OBJDUMP)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "mitkTestReleaseDebugSymbols: ${_required} is not set")
  endif()
endforeach()

execute_process(
  COMMAND "${MITK_DEBUG_SYMBOLS_OBJDUMP}" -h "${MITK_DEBUG_SYMBOLS_BINARY}"
  OUTPUT_VARIABLE _sections
  ERROR_VARIABLE _error
  RESULT_VARIABLE _result)

if(NOT _result EQUAL 0)
  message(FATAL_ERROR "mitkTestReleaseDebugSymbols: objdump failed for ${MITK_DEBUG_SYMBOLS_BINARY}: ${_error}")
endif()

if(NOT _sections MATCHES "\\.debug_line")
  message(FATAL_ERROR "mitkTestReleaseDebugSymbols: ${MITK_DEBUG_SYMBOLS_BINARY} has no .debug_line section; the Release debug symbol flags produced no DWARF")
endif()

message(STATUS "mitkTestReleaseDebugSymbols: ${MITK_DEBUG_SYMBOLS_BINARY} carries DWARF line tables")
