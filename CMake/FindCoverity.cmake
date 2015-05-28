# TODO: Search the 'known_installations' listed in ~/.coverity/coverity.conf
find_path(Coverity_DIR
  bin/cov-build
  PATHS /opt
  PATH_SUFFIXES cov-analysis-linux64-7.6.0 cov-analysis-linux64 cov-analysis coverity
  DOC "Path to the Coverity installation directory"
  )

if(Coverity_DIR)
  exec_program(${Coverity_DIR}/bin/cov-build
    ARGS "--ident"
    OUTPUT_VARIABLE coverity_ident_output
    RETURN_VALUE coverity_ident_return
    )
  if(coverity_ident_return)
    message(WARNING "The specified Coverity_DIR is not a valid Coverity installation directory")
    unset(Coverity_DIR CACHE)
  else()
    string(REGEX REPLACE "^Coverity .* version ([^ ]+) on .*" "\\1" Coverity_VERSION "${coverity_ident_output}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Coverity
  REQUIRED_VARS Coverity_DIR
  FOUND_VAR Coverity_FOUND
  VERSION_VAR Coverity_VERSION
  )
