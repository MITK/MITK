# Helpers for install and packaging scripts that inspect and repair the
# Mach-O binaries of an app bundle. Included by install(CODE) snippets and by
# FixMacOSInstaller.cmake; requires otool, install_name_tool and codesign.

# The generated install script sets no policy version, so the policies these
# helpers and their callers rely on are set here.
# Do not descend into symlinked directories, such as Versions/Current of a
# framework, which would list the same binaries twice.
cmake_policy(SET CMP0009 NEW)
# if(IN_LIST)
cmake_policy(SET CMP0057 NEW)

# Lists the Mach-O files below a directory, skipping symlinks.
function(mitk_macho_files out_var dir)
  file(GLOB_RECURSE _candidates LIST_DIRECTORIES FALSE "${dir}/*")
  set(_files "")
  foreach(_file IN LISTS _candidates)
    if(IS_SYMLINK "${_file}")
      continue()
    endif()
    file(READ "${_file}" _magic LIMIT 4 HEX)
    if(_magic STREQUAL "cffaedfe" OR _magic STREQUAL "cafebabe")
      list(APPEND _files "${_file}")
    endif()
  endforeach()
  set(${out_var} "${_files}" PARENT_SCOPE)
endfunction()

# Reads the load commands of a Mach-O file: the dylibs it depends on (weak
# ones separately), its LC_RPATH entries, its own install name and its
# deployment target.
function(mitk_macho_read file)
  cmake_parse_arguments(PARSE_ARGV 1 _arg "" "ID;MINOS" "DEPENDENCIES;WEAK_DEPENDENCIES;RPATHS")
  execute_process(COMMAND otool -l "${file}" OUTPUT_VARIABLE _output RESULT_VARIABLE _result)
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "otool -l failed on ${file}")
  endif()
  string(REPLACE "\n" ";" _lines "${_output}")
  set(_command "")
  set(_dependencies "")
  set(_weak_dependencies "")
  set(_rpaths "")
  set(_id "")
  set(_minos "")
  foreach(_line IN LISTS _lines)
    string(STRIP "${_line}" _line)
    if(_line MATCHES "^cmd (.+)$")
      set(_command "${CMAKE_MATCH_1}")
    elseif(_line MATCHES "^name (.+) \\(offset [0-9]+\\)$")
      set(_name "${CMAKE_MATCH_1}")
      if(_command STREQUAL "LC_LOAD_DYLIB" OR _command STREQUAL "LC_REEXPORT_DYLIB" OR _command STREQUAL "LC_LAZY_LOAD_DYLIB")
        list(APPEND _dependencies "${_name}")
      elseif(_command STREQUAL "LC_LOAD_WEAK_DYLIB")
        list(APPEND _weak_dependencies "${_name}")
      elseif(_command STREQUAL "LC_ID_DYLIB")
        set(_id "${_name}")
      endif()
    elseif(_line MATCHES "^path (.+) \\(offset [0-9]+\\)$" AND _command STREQUAL "LC_RPATH")
      list(APPEND _rpaths "${CMAKE_MATCH_1}")
    elseif(_line MATCHES "^minos (.+)$" AND _command STREQUAL "LC_BUILD_VERSION" AND NOT _minos)
      set(_minos "${CMAKE_MATCH_1}")
    elseif(_line MATCHES "^version (.+)$" AND _command STREQUAL "LC_VERSION_MIN_MACOSX" AND NOT _minos)
      set(_minos "${CMAKE_MATCH_1}")
    endif()
  endforeach()
  foreach(_kind ID MINOS DEPENDENCIES WEAK_DEPENDENCIES RPATHS)
    if(_arg_${_kind})
      string(TOLOWER "${_kind}" _local)
      set(${_arg_${_kind}} "${_${_local}}" PARENT_SCOPE)
    endif()
  endforeach()
endfunction()

# Expands a leading @loader_path or @executable_path of a path.
function(mitk_macho_expand out_var path loader executable_dir)
  get_filename_component(_loader_dir "${loader}" DIRECTORY)
  if(path MATCHES "^@loader_path(.*)$")
    set(path "${_loader_dir}${CMAKE_MATCH_1}")
  elseif(path MATCHES "^@executable_path(.*)$")
    set(path "${executable_dir}${CMAKE_MATCH_1}")
  endif()
  cmake_path(NORMAL_PATH path)
  set(${out_var} "${path}" PARENT_SCOPE)
endfunction()

# Re-signs ad hoc every binary of a list whose signature is invalid. Changing
# a load command invalidates the signature, and arm64 kills a process that
# loads such a binary. Depending on its version, macdeployqt signs what it
# changes or not, so this runs after all changes. The final signing of a
# package replaces these signatures.
#
# Each binary is signed as a copy under another name: codesign treats the main
# executable of a bundle as the bundle itself and then fails on the other
# files in Contents/MacOS.
function(mitk_macho_resign_invalid)
  foreach(_file IN LISTS ARGN)
    execute_process(COMMAND codesign --verify "${_file}" RESULT_VARIABLE _result OUTPUT_QUIET ERROR_QUIET)
    if(_result EQUAL 0)
      continue()
    endif()
    set(_copy "${_file}.mitk-resign")
    file(COPY_FILE "${_file}" "${_copy}")
    execute_process(COMMAND codesign --force --sign - "${_copy}" RESULT_VARIABLE _result ERROR_VARIABLE _error OUTPUT_QUIET)
    if(NOT _result EQUAL 0)
      file(REMOVE "${_copy}")
      message(FATAL_ERROR "codesign failed on ${_file}: ${_error}")
    endif()
    file(RENAME "${_copy}" "${_file}")
  endforeach()
endfunction()
