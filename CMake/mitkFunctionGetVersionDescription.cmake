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
#! a suffix [local changes]
#!
#! \param source_dir The directory containing a working copy
#! \param prefix A prefix to prepent to the variables containing
#!               the extracted information
#!
function(mitkFunctionGetVersionDescription source_dir prefix)

  if(NOT prefix)
    message(FATAL_ERROR "prefix argument not specified")
  endif()

  # initialize variable
  set(_wc_description "NO TAG FOUND")
  set(_dirty_repo_str " [local changes]")

  find_package(Git)
  if(GIT_FOUND)

    GIT_IS_REPO(${source_dir} _is_git_repo)
    if(_is_git_repo)
      execute_process(COMMAND git describe --exact-match --dirty=${_dirty_repo_str}
                      WORKING_DIRECTORY ${source_dir}
                      OUTPUT_VARIABLE _project_git_tagname
                      RESULT_VARIABLE _proper_version
                      ERROR_VARIABLE _description_error )

      if(_proper_version EQUAL 0 )
        set(_wc_description ${_project_git_tagname})

      else(_proper_version EQUAL 0)
        # the execution failed, i.e. the HEAD has no tag,
        # for fallback string: execute again but without the --exact-match
        execute_process(COMMAND git describe --dirty=${_dirty_repo_str}
                    WORKING_DIRECTORY ${source_dir}
                    OUTPUT_VARIABLE _wc_description
                    RESULT_VARIABLE _proper_version
                    ERROR_VARIABLE _description_error)
      endif()

      # remove newline at and of the string
      string(STRIP "${_wc_description}" _wc_description)

    endif()
  endif()

  set(${prefix}_REVISION_DESC ${_wc_description} PARENT_SCOPE )

endfunction()
