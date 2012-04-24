macro(GetLinuxDistribution)

execute_process(COMMAND lsb_release -a OUTPUT_VARIABLE _out ERROR_VARIABLE _err RESULT_VARIABLE _result)

string(COMPARE EQUAL "${_out}" "" _ok)

if(_ok)
  string(REGEX REPLACE "\n" ";" _out ${_err})
else()
  string(REGEX REPLACE "\n" ";" _out ${_out})
endif()

foreach(_i ${_out})

  if("${_i}" MATCHES "^[Rr][Ee][Ll][Ee][Aa][Ss][Ee].*")
    string(REGEX REPLACE ":" ";" _i ${_i})
    list(GET _i -1 _i)
    string(STRIP ${_i} _release)
  endif()

  if("${_i}" MATCHES "^[Dd][Ii][Ss][Tt][Rr][Ii][Bb][Uu][Tt][Oo][Rr].*")
    string(REGEX REPLACE ":" ";" _i ${_i})
    list(GET _i -1 _i)
    string(STRIP ${_i} _distrib)
  endif()

endforeach(_i)

string(COMPARE EQUAL "${_distrib}" "" _ok1)
string(COMPARE EQUAL "${_release}" "" _ok2)

if(${_ok1} OR ${_ok2})
  message(SEND_ERROR "could not determine linux release!")
endif()

set(LINUX_DISTRIBUTION ${_distrib})
set(LINUX_RELEASE ${_release})

endmacro()
