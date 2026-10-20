# Boost.Process 1.89 to 1.92: BOOST_PROCESS_V2_DISABLE_SIGNALSET does not
# compile with the signal-based process handle (macOS). The wait operation
# for the disabled signal set kept two members and the old completion when
# async_wait() changed to take the exit code (boostorg/process 31d6b5c).
# MitkPythonWorker needs the define, so Boost does not replace the SIGCHLD
# handler QProcess relies on.
#
# This is the fix proposed upstream: the same members as with a signal set,
# and async_wait() completes with operation_not_supported. Drop this once
# Boost ships it.

set(_file "${BOOST_SRC}/libs/process/include/boost/process/v2/detail/process_handle_signal.hpp")

if(NOT EXISTS "${_file}")
  return()
endif()

file(READ "${_file}" _contents)

set(_marker "[MITK] DISABLE_SIGNALSET fix")

if(_contents MATCHES "\\[MITK\\] DISABLE_SIGNALSET fix")
  return()
endif()

# From the members of the branch to the end of its operator()(Self&&). An
# earlier version of this patch only added a member in between.
set(_begin_anchor "signal_set_dummy_ dummy_;")
set(_end_anchor "native_exit_code_type(), ec));")

string(FIND "${_contents}" "${_begin_anchor}" _begin)
string(FIND "${_contents}" "${_end_anchor}" _end)

if(_begin EQUAL -1 OR _end EQUAL -1 OR _end LESS _begin)
  message(FATAL_ERROR "Boost: the anchor of ${CMAKE_CURRENT_LIST_FILE} is gone. "
    "Check whether Boost.Process fixed BOOST_PROCESS_V2_DISABLE_SIGNALSET and drop the patch.")
endif()

# The closing brace of operator()(Self&&) follows the end anchor.
string(SUBSTRING "${_contents}" ${_end} -1 _rest)
string(FIND "${_rest}" "}" _brace)
math(EXPR _replaced_end "${_end} + ${_brace} + 1")

string(SUBSTRING "${_contents}" 0 ${_begin} _head)
string(SUBSTRING "${_contents}" ${_replaced_end} -1 _tail)

set(_fixed [=[signal_set_dummy_ &handle; // @MARKER@
        pid_type pid_;
        native_exit_code_type & exit_code;

        template<typename Self>
        void operator()(Self &&self)
        {
          auto exec = net::get_associated_immediate_executor(self, handle.get_executor());
          error_code ec;
          BOOST_PROCESS_V2_ASSIGN_EC(ec, net::error::operation_not_supported);
          net::dispatch(exec, net::append(std::move(self), ec));
        }]=])

string(REPLACE "@MARKER@" "${_marker}" _fixed "${_fixed}")

file(WRITE "${_file}" "${_head}${_fixed}${_tail}")
message(STATUS "Boost: fixed BOOST_PROCESS_V2_DISABLE_SIGNALSET in Boost.Process")
