macro(GetLinuxDistribution)

if( UNIX )

execute_process(COMMAND lsb_release -a OUTPUT_VARIABLE _out)
message("********************")

string(REPLACE "\\n" ";" _out ${_out})

foreach(_i ${_out})
    message("************")
    message("${_i}")
  if("${_i}" MATCHES "^[Rr][Ee][Ll][Ee][Aa][Ss][Ee].*")
    message("1release: ${_i}")
    string(REPLACE ":" ";" _i ${_i})
    message("2release: ${_i}")
    list(GET ${_i} -1 _i)
    message("3release: ${_i}")
    string(STRIP ${_i} _i)
    message("4release: ${_i}")
  endif()
endforeach(_i)

#string(REGEX REPLACE ".*Distributor ID:[^0-9a-zA-Z]*([0-9a-zA-Z]+).*" "\\1" dis_id "${test}")
#message("dis_id: ${dis_id}")

#string(REGEX REPLACE ".*Release:[ \\t]*(.*)\\n+.*" "\\1" release "${test}")
# ([0-9]+.[0-9]+).*" "\\1" release "${test}")
#message("release: ${release}")

SET(LINUX_DISTRIBUTION ${dis_id})
SET(LINUX_RELEASE ${release})

endif()

endmacro()