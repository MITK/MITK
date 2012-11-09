#
# Use to attach a CTest/CDash label to a test (already added via ADD_TEST)
#
# Call with or without an explicit second parameter. If no second parameter is given,
# ${MITK_DEFAULT_SUBPROJECTS} will be used as standard label
#
function(mitkFunctionAddTestLabel test_name)
  set(label ${MITK_DEFAULT_SUBPROJECTS})
  if(ARGN)
    set(label ${ARGN})
  endif()
  set_property(TEST ${test_name} APPEND PROPERTY LABELS ${label})
endfunction(mitkFunctionAddTestLabel)


