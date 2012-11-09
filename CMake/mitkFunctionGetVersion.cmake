#! \brief Extract version information from a local working copy
#!
#! If the source_dir variable points to a git repository, this function
#! extracts the current revision hash and branch/tag name.
#!
#! If the source_dir variable points to a subversion repository, this
#! function extracts the current svn revision.
#
#! The information is provided in
#! \ul
#! \li ${prefix}_REVISION_ID The git hash or svn revision value
#! \li ${prefix}_REVISION_NAME The git branch/tag name or empty
#! \li ${prefix}_WC_TYPE The working copy type, one of "local", "git", or "svn"
#! \lu
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


  find_package(Git)
  if(GIT_FOUND)

    GIT_IS_REPO(${source_dir} _is_git_repo)
    if(_is_git_repo)
      set(_wc_type "git")
      GIT_WC_INFO(${source_dir} ${prefix})

      set(_wc_id ${${prefix}_WC_REVISION_HASH})

      string(REPLACE " " ";" hash_name ${${prefix}_WC_REVISION_NAME})
      list(GET hash_name 1 name)
      if(name)
        set(_wc_name ${name})
      endif()
    endif()
  endif()

  # test for svn working copy
  if(_wc_type STREQUAL "local")

    find_package(Subversion)
    if(Subversion_FOUND)
      execute_process(COMMAND ${Subversion_SVN_EXECUTABLE} info
                      WORKING_DIRECTORY ${source_dir}
                      RESULT_VARIABLE _subversion_result
                      OUTPUT_QUIET
                      ERROR_QUIET)

      if(NOT _subversion_result)
        set(_wc_type svn)
        Subversion_WC_INFO(${source_dir} ${prefix})
        set(_wc_id ${${prefix}_WC_REVISION})
      endif()
    endif()

  endif()

  set(${prefix}_WC_TYPE ${_wc_type} PARENT_SCOPE)
  set(${prefix}_REVISION_ID ${_wc_id} PARENT_SCOPE)
  set(_shortid ${_wc_id})
  if(_wc_type STREQUAL "git")
    string(SUBSTRING ${_shortid} 0 8 _shortid)
  endif()
  set(${prefix}_REVISION_SHORTID ${_shortid} PARENT_SCOPE)
  set(${prefix}_REVISION_NAME ${_wc_name} PARENT_SCOPE)

  # For backwards compatibility
  set(${prefix}_WC_REVISION_HASH ${_wc_id} PARENT_SCOPE)
  set(${prefix}_WC_REVISION_NAME ${_wc_name} PARENT_SCOPE)

endfunction()
