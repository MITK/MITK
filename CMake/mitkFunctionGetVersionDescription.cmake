#! \brief Extract the version description from a local working copy
#!
#! If the given repository is a git repository, the functions runs the
#! git rev-parse --exact-match HEAD command
#!
#! Information provided is stored in  ${prefix}_REVISION_DESC an is
#! \ul
#!  \li The exact tag if the HEAD of the source-tree has a tag
#!  \li the 'git describe' output, which is <latest-reachable-tag>-<#Commits>-g<SHORT-HASH>
#! \lu
#! In case the working copy contains local changes, the ${prefix}_REVISION_DESC strings will contain
#! a suffix [local changes].
#!
#! The revision description can be overridden by a ${prefix}_CUSTOM_REVISION_DESC variable.
#!
#! \param source_dir The directory containing a working copy
#! \param prefix A prefix to prepent to the variables containing
#!               the extracted information
#!
function(mitkFunctionGetVersionDescription source_dir prefix)

  if(NOT prefix)
    message(FATAL_ERROR "prefix argument not specified")
  endif()

  if(${prefix}_CUSTOM_REVISION_DESC)
    set(_wc_description ${${prefix}_CUSTOM_REVISION_DESC})
  else()
    # initialize variable
    set(_wc_description "unknown_version")
    set(_dirty_repo_str "-local_changes")

    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --is-inside-work-tree
      WORKING_DIRECTORY "${source_dir}"
      RESULT_VARIABLE _result_var
      OUTPUT_QUIET
      ERROR_QUIET)
    if(NOT _result_var)
      execute_process(COMMAND ${GIT_EXECUTABLE} describe --exact-match --dirty=${_dirty_repo_str}
                      WORKING_DIRECTORY ${source_dir}
                      OUTPUT_VARIABLE _project_git_tagname
                      RESULT_VARIABLE _proper_version
                      ERROR_VARIABLE _error_msg)

      if(_proper_version EQUAL 0)
        set(_wc_description ${_project_git_tagname})
      else()
        # the execution failed, i.e. the HEAD has no tag,
        # for fallback string: execute again but without the --exact-match
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --dirty=${_dirty_repo_str}
                        WORKING_DIRECTORY ${source_dir}
                        OUTPUT_VARIABLE _wc_description
                        RESULT_VARIABLE _proper_version
                        ERROR_VARIABLE _error_msg)

        if(NOT _proper_version EQUAL 0)
          # last fallback, i.e. working copy is a shallow clone, at least use
          # commit hash
          execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --dirty=${_dirty_repo_str}
                          WORKING_DIRECTORY ${source_dir}
                          OUTPUT_VARIABLE _wc_description)
        endif()
      endif()
      # remove newline at and of the string
      string(STRIP "${_wc_description}" _wc_description)
    endif()
  endif()

  set(${prefix}_REVISION_DESC ${_wc_description} PARENT_SCOPE)

endfunction()
