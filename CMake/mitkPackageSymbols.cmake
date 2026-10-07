# Build-time helper invoked via "cmake -P" by the package-symbols target.
#
# Archives MITK's own debug symbols from the build-tree runtime output
# directory. Third-party and prebuilt dependencies live under the superbuild
# ep/ prefix, not here, so sourcing from the build tree keeps the archive
# MITK-only. Globbing happens here, at build time, because the symbol files
# only exist after the binaries have been linked.
#
# Symbols are staged flat (by base name) before archiving, so a debugger can
# resolve them with a single non-recursive symbol path (e.g. cdb -y <dir>).
# Target output names are unique across MITK, so base names do not collide.
#
# Required cache-style variables (passed with -D):
#   MITK_SYMBOL_RUNTIME_DIR  runtime output directory (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
#   MITK_SYMBOL_OUTPUT       absolute path of the archive to create
#   MITK_SYMBOL_STAGING_DIR  scratch directory for staging
# Optional:
#   MITK_SYMBOL_CONFIG       build configuration (multi-config generators)
#   MITK_SYMBOL_LIBRARY_DIR  library output directory (CMAKE_LIBRARY_OUTPUT_DIRECTORY).
#                            Where the shared libraries are on ELF and Mach-O;
#                            on Windows the DLLs are runtime artifacts instead.

foreach(_required MITK_SYMBOL_RUNTIME_DIR MITK_SYMBOL_OUTPUT MITK_SYMBOL_STAGING_DIR)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "mitkPackageSymbols: ${_required} is not set")
  endif()
endforeach()

# Non-shippable test binaries; their symbols do not belong in a release archive.
set(_exclude_regex "TestDriver|Test(ing)?Helper")

# Per-config runtime dir: multi-config generators nest binaries in <dir>/<config>,
# single-config ones write straight into <dir>.
set(_bin_dir "${MITK_SYMBOL_RUNTIME_DIR}")
set(_per_config_layout FALSE)
if(MITK_SYMBOL_CONFIG AND EXISTS "${MITK_SYMBOL_RUNTIME_DIR}/${MITK_SYMBOL_CONFIG}")
  set(_bin_dir "${MITK_SYMBOL_RUNTIME_DIR}/${MITK_SYMBOL_CONFIG}")
  set(_per_config_layout TRUE)
endif()

set(_lib_dir "${MITK_SYMBOL_LIBRARY_DIR}")
if(_lib_dir AND MITK_SYMBOL_CONFIG AND EXISTS "${_lib_dir}/${MITK_SYMBOL_CONFIG}")
  set(_lib_dir "${_lib_dir}/${MITK_SYMBOL_CONFIG}")
endif()

file(REMOVE_RECURSE "${MITK_SYMBOL_STAGING_DIR}")
file(MAKE_DIRECTORY "${MITK_SYMBOL_STAGING_DIR}")

set(_staged 0)

# In script mode (cmake -P) key off the CMAKE_HOST_* variables.
if(CMAKE_HOST_WIN32)
  # MSVC emits one .pdb next to each linked DLL/EXE (bin/<config>,
  # bin/plugins/<config>, ...). Globbing starts at the runtime dir rather than
  # _bin_dir because the plugin output is a sibling subtree; in a per-config
  # layout the config filter is what keeps the other configs out of the archive.
  file(GLOB_RECURSE _pdbs "${MITK_SYMBOL_RUNTIME_DIR}/*.pdb")
  foreach(_pdb ${_pdbs})
    if(_per_config_layout AND NOT _pdb MATCHES "/${MITK_SYMBOL_CONFIG}/")
      continue()
    endif()
    if(_pdb MATCHES "${_exclude_regex}")
      continue()
    endif()
    file(COPY "${_pdb}" DESTINATION "${MITK_SYMBOL_STAGING_DIR}")
    math(EXPR _staged "${_staged} + 1")
  endforeach()
else()
  # On ELF the unstripped build-tree binary is the symbol file: it carries
  # the DWARF that strip removes from the shipped copy, so archive it as is.
  # On macOS the linker leaves the DWARF in the object files, so dsymutil
  # packs it into a self-contained .dSYM here. Best effort - warns and skips
  # if dsymutil is unavailable.
  if(CMAKE_HOST_APPLE)
    find_program(_dsymutil dsymutil)
  endif()

  set(_search_dirs "${_bin_dir}")
  if(_lib_dir AND NOT _lib_dir STREQUAL _bin_dir)
    list(APPEND _search_dirs "${_lib_dir}")
  endif()

  set(_binaries "")
  foreach(_dir ${_search_dirs})
    if(CMAKE_HOST_APPLE)
      file(GLOB_RECURSE _libraries "${_dir}/*.dylib" "${_dir}/*.so")
    else()
      file(GLOB_RECURSE _libraries "${_dir}/*.so")
    endif()
    list(APPEND _binaries ${_libraries})

    # Application executables carry no extension on ELF and Mach-O, so the
    # library globs above miss them and their frames would stay unresolved.
    file(GLOB_RECURSE _candidates "${_dir}/*")
    foreach(_candidate ${_candidates})
      get_filename_component(_candidate_name "${_candidate}" NAME)
      if(NOT _candidate_name MATCHES "\\.")
        list(APPEND _binaries "${_candidate}")
      endif()
    endforeach()
  endforeach()

  list(REMOVE_DUPLICATES _binaries)

  foreach(_binary ${_binaries})
    if(_binary MATCHES "${_exclude_regex}")
      continue()
    endif()
    get_filename_component(_name "${_binary}" NAME)
    if(CMAKE_HOST_APPLE AND _dsymutil)
      execute_process(COMMAND "${_dsymutil}" "${_binary}" -o "${MITK_SYMBOL_STAGING_DIR}/${_name}.dSYM"
        RESULT_VARIABLE _result)
      if(_result EQUAL 0)
        math(EXPR _staged "${_staged} + 1")
      endif()
    elseif(NOT CMAKE_HOST_APPLE)
      file(COPY "${_binary}" DESTINATION "${MITK_SYMBOL_STAGING_DIR}")
      math(EXPR _staged "${_staged} + 1")
    endif()
  endforeach()
endif()

if(_staged EQUAL 0)
  message(FATAL_ERROR "mitkPackageSymbols: no symbol files found under ${MITK_SYMBOL_RUNTIME_DIR}")
endif()

file(GLOB _staged_entries RELATIVE "${MITK_SYMBOL_STAGING_DIR}" "${MITK_SYMBOL_STAGING_DIR}/*")

file(REMOVE "${MITK_SYMBOL_OUTPUT}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E tar cf "${MITK_SYMBOL_OUTPUT}" --format=zip -- ${_staged_entries}
  WORKING_DIRECTORY "${MITK_SYMBOL_STAGING_DIR}"
  RESULT_VARIABLE _tar_result)

if(NOT _tar_result EQUAL 0)
  message(FATAL_ERROR "mitkPackageSymbols: failed to create ${MITK_SYMBOL_OUTPUT}")
endif()

message(STATUS "mitkPackageSymbols: archived ${_staged} symbol file(s) into ${MITK_SYMBOL_OUTPUT}")
