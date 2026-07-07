function(mitkFunctionOrganizeSources)

  # This function discovers header and template files corresponding to
  # the given source files and organizes everything into IDE source groups.
  #
  # For each .cpp/.c source file, it looks for a matching .h file:
  #   1. Next to the source file (e.g. src/Foo.cpp -> src/Foo.h)
  #   2. In the include/ directory (e.g. src/Foo.cpp -> include/Foo.h)
  #
  # For each .h file (both explicitly listed and auto-discovered), it
  # looks for a matching .tpp file in the same directory.
  #
  # Output variables (set in parent scope):
  #   CORRESPONDING__H_FILES   - auto-discovered header files
  #   CORRESPONDING__TPP_FILES - auto-discovered template implementation files
  #   GLOBBED__H_FILES         - all headers found via glob (header-only modules)

  cmake_parse_arguments(_ORG "" "" "HEADER;SOURCE;META;UI;QRC" ${ARGN})

  set(CORRESPONDING__H_FILES "")
  set(CORRESPONDING__TPP_FILES "")
  set(GLOBBED__H_FILES "")

  if(_ORG_SOURCE)
    foreach(_file ${_ORG_SOURCE})
      string(REGEX REPLACE "(.*)\\.(cpp|c)$" "\\1.h" _H_FILE ${_file})

      if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_H_FILE}")
        list(APPEND CORRESPONDING__H_FILES "${_H_FILE}")
      endif()
      # Also check in include/ directory (e.g. src/Foo.cpp -> include/Foo.h)
      get_filename_component(_basename "${_H_FILE}" NAME)
      if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/${_basename}")
        list(APPEND CORRESPONDING__H_FILES "include/${_basename}")
      endif()
    endforeach()
  else()
    file(GLOB_RECURSE GLOBBED__H_FILES *.h)
  endif()

  # Discover .tpp files for all known headers (explicit + auto-discovered)
  set(_all_headers ${_ORG_HEADER} ${CORRESPONDING__H_FILES} ${GLOBBED__H_FILES})
  foreach(_header ${_all_headers})
    string(REGEX REPLACE "\\.h$" ".tpp" _TPP_FILE "${_header}")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_TPP_FILE}")
      list(APPEND CORRESPONDING__TPP_FILES "${_TPP_FILE}")
    endif()
  endforeach()

  if(CORRESPONDING__H_FILES)
    list(REMOVE_DUPLICATES CORRESPONDING__H_FILES)
  endif()

  if(CORRESPONDING__TPP_FILES)
    list(REMOVE_DUPLICATES CORRESPONDING__TPP_FILES)
  endif()

  set(CORRESPONDING__H_FILES ${CORRESPONDING__H_FILES} PARENT_SCOPE)
  set(CORRESPONDING__TPP_FILES ${CORRESPONDING__TPP_FILES} PARENT_SCOPE)
  set(GLOBBED__H_FILES ${GLOBBED__H_FILES} PARENT_SCOPE)

  # Organize into source groups for IDE display.
  # Headers in include/ are public, all others are private.
  source_group("Source Files" FILES ${_ORG_SOURCE})

  set(_public_headers "")
  set(_private_headers "")

  foreach(_header ${_ORG_HEADER} ${CORRESPONDING__H_FILES} ${GLOBBED__H_FILES} ${CORRESPONDING__TPP_FILES})
    if(_header MATCHES "^include/")
      list(APPEND _public_headers "${_header}")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/${_header}")
      list(APPEND _public_headers "${_header}")
    else()
      list(APPEND _private_headers "${_header}")
    endif()
  endforeach()

  if(_public_headers)
    source_group("Header Files/Public" FILES ${_public_headers})
  endif()
  if(_private_headers)
    source_group("Header Files/Private" FILES ${_private_headers})
  endif()

  if(_ORG_UI)
    source_group("Qt/UI Files" FILES ${_ORG_UI})
  endif()

  if(_ORG_QRC)
    source_group("Qt/Resource Files" FILES ${_ORG_QRC})
  endif()

  # Categorize auto-generated Qt files (from AUTOMOC/AUTOUIC/AUTORCC/CTK)
  source_group("Qt/Generated Files/MOC" REGULAR_EXPRESSION "(moc_|mocs_compilation_).*\\.cpp$")
  source_group("Qt/Generated Files/QRC" REGULAR_EXPRESSION "qrc_.*\\.cpp$")
  source_group("Qt/Generated Files/UI" REGULAR_EXPRESSION "ui_.*\\.h$")
  source_group("Qt/Generated Files/AUTOUIC" REGULAR_EXPRESSION "autouic_.*\\.stamp$")
  source_group("Qt/Generated Files/Resources" REGULAR_EXPRESSION "_(cached|manifest)\\.qrc$")

  if(_ORG_META)
    source_group("Plugin Files" FILES ${_ORG_META})
  endif()

endfunction()
