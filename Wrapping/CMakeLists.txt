find_package(SWIG QUIET)

if(SWIG_FOUND)
  include(mitkLanguageOptions)
  include(UseSWIG)

  include(mitkSwigAddLibraryDependencies)
  include(mitkSwigPrepareFiles)

  # Path to common files
  set(MITK_WRAPPING_COMMON_DIR ${MITK_SOURCE_DIR}/Wrapping/Common)
  # make a manual list of dependencies for the Swig.i files
  list( APPEND SWIG_EXTRA_DEPS
    "${MITK_WRAPPING_COMMON_DIR}/MITK_Common.i"
    )

  # A general packaging target, not built by default, to build packages for each
  # language.  This should depend on all language specific targets.

  add_custom_target( dist ${CMAKE_COMMAND} -E echo "Finished generating wrapped packages for distribution..." )
  set_property(TARGET dist PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Wrapping")

  #
  # lua SWIG configuration
  #
  #if ( WRAP_LUA )
  #  add_subdirectory ( Lua )
  #endif()

  #
  # python SWIG configuration
  #
  if ( WRAP_PYTHON )
    add_subdirectory ( Python )
  endif()

  #
  # ruby SWIG configuration
  #
  #if ( WRAP_RUBY )
  #  add_subdirectory ( Ruby )
  #endif()

  #
  # JAVA SWIG configuration
  #
  #if ( WRAP_JAVA )
  #  add_subdirectory( Java )
  #endif()

  #
  # C# SWIG configuration
  #
  #if ( WRAP_CSHARP )
  #  add_subdirectory ( CSharp )
  #endif()

  #
  # TCL SWIG configuration
  #
  #if ( WRAP_TCL )
  #  add_subdirectory ( Tcl )
  #endif()

  #
  # R SWIG configuration
  #
  #if ( WRAP_R )
  #  add_subdirectory( R )
  #endif()
endif()
