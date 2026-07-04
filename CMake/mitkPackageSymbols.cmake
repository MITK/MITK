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

foreach(_required MITK_SYMBOL_RUNTIME_DIR MITK_SYMBOL_OUTPUT MITK_SYMBOL_STAGING_DIR)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "mitkPackageSymbols: ${_required} is not set")
  endif()
endforeach()

# Non-shippable test binaries; their symbols do not belong in a release archive.
set(_exclude_regex "TestDriver|Test(ing)?Helper")

# Per-config runtime dir: multi-config generators nest binaries in <dir>/<config>.
set(_bin_dir "${MITK_SYMBOL_RUNTIME_DIR}")
if(MITK_SYMBOL_CONFIG AND EXISTS "${MITK_SYMBOL_RUNTIME_DIR}/${MITK_SYMBOL_CONFIG}")
  set(_bin_dir "${MITK_SYMBOL_RUNTIME_DIR}/${MITK_SYMBOL_CONFIG}")
endif()

file(REMOVE_RECURSE "${MITK_SYMBOL_STAGING_DIR}")
file(MAKE_DIRECTORY "${MITK_SYMBOL_STAGING_DIR}")

set(_staged 0)

# In script mode (cmake -P) key off the CMAKE_HOST_* variables.
if(CMAKE_HOST_WIN32)
  # MSVC emits one .pdb next to each linked DLL/EXE (bin/<config>,
  # bin/plugins/<config>, ...). Collect and flatten them.
  file(GLOB_RECURSE _pdbs "${MITK_SYMBOL_RUNTIME_DIR}/*.pdb")
  foreach(_pdb ${_pdbs})
    if(MITK_SYMBOL_CONFIG AND NOT _pdb MATCHES "/${MITK_SYMBOL_CONFIG}/")
      continue()
    endif()
    if(_pdb MATCHES "${_exclude_regex}")
      continue()
    endif()
    file(COPY "${_pdb}" DESTINATION "${MITK_SYMBOL_STAGING_DIR}")
    math(EXPR _staged "${_staged} + 1")
  endforeach()
else()
  # Split DWARF (.dwo) lives beside the objects, not next to the binaries, so
  # pack each binary's debug info into a self-contained sidecar: dwp (.dwp) on
  # ELF, dsymutil (.dSYM) on macOS. Best effort - warns and skips if the tool
  # is unavailable (e.g. the -g fallback leaves debug info embedded instead).
  if(CMAKE_HOST_APPLE)
    file(GLOB_RECURSE _binaries "${_bin_dir}/*.dylib" "${_bin_dir}/*.so")
    find_program(_dsymutil dsymutil)
  else()
    file(GLOB_RECURSE _binaries "${_bin_dir}/*.so")
    find_program(_dwp NAMES dwp)
  endif()

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
    elseif(NOT CMAKE_HOST_APPLE AND _dwp)
      execute_process(COMMAND "${_dwp}" -e "${_binary}" -o "${MITK_SYMBOL_STAGING_DIR}/${_name}.dwp"
        RESULT_VARIABLE _result)
      if(_result EQUAL 0 AND EXISTS "${MITK_SYMBOL_STAGING_DIR}/${_name}.dwp")
        math(EXPR _staged "${_staged} + 1")
      endif()
    endif()
  endforeach()

  if(_staged EQUAL 0)
    message(WARNING "mitkPackageSymbols: no debug symbols collected. If dwp/dsymutil "
                    "is unavailable, debug info is embedded in the binaries themselves.")
  endif()
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
