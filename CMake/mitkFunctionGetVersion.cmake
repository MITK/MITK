#! \brief Extract version information from a local working copy
#!
#! If the source_dir variable points to a git repository, this function
#! extracts the current revision hash and branch/tag name.
#
#! The information is provided in
#! <ul>
#!  <li> ${prefix}_REVISION_ID The git hash
#!  <li> ${prefix}_REVISION_SHORTID The short git hash (8 digits)
#!  <li> ${prefix}_REVISION_NAME The git branch/tag name or empty
#!  <li> ${prefix}_WC_TYPE The working copy type, one of "local", or "git"
#! </ul>
#!
#! \param source_dir The directory containing a working copy
#! \param prefix A prefix to prepend to the variables containing
#!               the extracted information.
#!
function(mitkFunctionGetVersion source_dir prefix)

  if(NOT prefix)
    message(FATAL_ERROR "prefix argument not specified")
  endif()

  # initialize variables
  set(_wc_type "local")
  set(_wc_id "")
  set(_wc_name "")

  execute_process(COMMAND ${GIT_EXECUTABLE} rev-list -1 HEAD
    WORKING_DIRECTORY "${source_dir}"
    RESULT_VARIABLE _result_var
    OUTPUT_VARIABLE ${prefix}_WC_REVISION_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)

  if(NOT _result_var)
    set(_wc_type "git")

    execute_process(COMMAND ${GIT_EXECUTABLE} name-rev --name-only ${${prefix}_WC_REVISION_HASH}
      WORKING_DIRECTORY "${source_dir}"
      OUTPUT_VARIABLE ${prefix}_WC_REVISION_NAME
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(_wc_id ${${prefix}_WC_REVISION_HASH})
    set(_wc_name ${${prefix}_WC_REVISION_NAME})
  endif()

  set(${prefix}_WC_TYPE ${_wc_type} PARENT_SCOPE)
  set(${prefix}_REVISION_ID ${_wc_id} PARENT_SCOPE)
  set(_shortid ${_wc_id})
  if(_wc_type STREQUAL "git")
    string(SUBSTRING ${_shortid} 0 8 _shortid)
  endif()
  set(${prefix}_REVISION_SHORTID ${_shortid} PARENT_SCOPE)
  set(${prefix}_REVISION_NAME ${_wc_name} PARENT_SCOPE)

endfunction()
